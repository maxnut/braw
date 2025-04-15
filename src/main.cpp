#include "codegen/x86-64/code_generator.hpp"
#include "codegen/x86-64/emitter.hpp"
#include "codegen/x86-64/file.hpp"
#include "ir/file.hpp"
#include "ir/from_ssa/builder_ssa.hpp"
#include "lexer/lexer.hpp"
#include "macro/evaluator.hpp"
#include "parser/parser.hpp"
#include "semantic-analyzer/semantic_analyzer.hpp"
#include "ir/printer/ir_printer.hpp"
#include "ssa/builder.hpp"
#include "ssa/file.hpp"
#include "ssa/printer.hpp"
#include "utils.hpp"

#include <spdlog/spdlog.h>
#include <args/args.hxx>

#include <filesystem>
#include <fstream>

int main(int argc, char** argv) {
    spdlog::set_pattern("[%^%l%$] %v");
    spdlog::set_level(spdlog::level::debug);

    args::ArgumentParser parser("Braw Compiler - A simple compiler for the Braw programming language.");
    args::HelpFlag help(parser, "help", "Displays this help menu", {'h', "help"});
    args::Positional<std::string> inputFile(parser, "file", "The source file to compile");
    args::ValueFlag<std::string> outputDirectory(parser, "output", "The directory to output to", {'o', "output"}, "out.asm");
    args::Flag assemble(parser, "assemble", "Assemble the output file", {"assemble"});
    args::Flag link(parser, "link", "Link the output file", {'l', "link"});
    args::Flag debug(parser, "debug", "Add debug information", {'d', "debug"});

    try {
        parser.ParseCLI(argc, argv);
    } catch (const args::Help&) {
        std::cout << parser << std::endl;
        return 0;
    } catch (const args::ParseError& e) {
        spdlog::error("{}", e.what());
        return 1;
    }

    if (!inputFile) {
        spdlog::error("No file specified. Use --help for usage.");
        return 1;
    }

    if(!std::filesystem::exists(Utils::getStdPath())) {
        spdlog::warn("Standard library not found. Set the \"BRAW_STDLIB\" environment variable or place it in the current directory.");
    }

    std::filesystem::path filepath(inputFile.Get());
    std::filesystem::path outputPath = outputDirectory.Get();
    std::filesystem::create_directories(outputPath);

    auto tokens = Lexer::tokenize(filepath);
    if (!tokens) {
        //TODO: add proper error return to lexer
        spdlog::error("Failed to tokenize file");
        return 1;
    }

    auto ast = Parser::parse(tokens.value(), filepath);
    if (!ast) {
        spdlog::error("{}({},{}): ParseError: {}", ast.error().m_path.string(), ast.error().m_line, ast.error().m_column, ast.error().m_message);
        return 1;
    }

    auto ctxOr = SemanticAnalyzer::fillTypes(ast.value().get());
    if (!ctxOr) {
        SemanticError err = ctxOr.error();
        spdlog::error("{}({},{}): SemanticError: {}\n{}", err.m_path.string(), err.m_rangeBegin.first, err.m_rangeBegin.second, err.m_message, Utils::extractRangeWithContext(err.m_path, err.m_rangeBegin.first, err.m_rangeBegin.second, err.m_rangeEnd.first, err.m_rangeEnd.second));
        return 1;
    }

    std::optional<Macro::MacroError> macroErr = Macro::Evaluator::processAST(ast.value(), ctxOr.value());

    if(macroErr) {
        spdlog::error("{}({},{}): MacroError: {}\n{}", macroErr->m_path.string(), macroErr->m_rangeBegin.first, macroErr->m_rangeBegin.second, macroErr->m_message, Utils::extractRangeWithContext(macroErr->m_path, macroErr->m_rangeBegin.first, macroErr->m_rangeBegin.second, macroErr->m_rangeEnd.first, macroErr->m_rangeEnd.second));
        return 1;
    }
    
    std::optional<SemanticError> err = SemanticAnalyzer::analyze(ast.value().get(), ctxOr.value());
    if(err) {
        spdlog::error("{}({},{}): SemanticError: {}\n{}", err->m_path.string(), err->m_rangeBegin.first, err->m_rangeBegin.second, err->m_message, Utils::extractRangeWithContext(err->m_path, err->m_rangeBegin.first, err->m_rangeBegin.second, err->m_rangeEnd.first, err->m_rangeEnd.second));
        return 1;
    }

    BrawContext ctx = ctxOr.value();
    ctx.m_debug = debug;

    std::vector<SSA::File> ssaFiles = SSA::Builder::build(ast.value().get(), ctx);
    for(const SSA::File& file : ssaFiles) {
        bool allExt = true;
        for(auto& f : file.m_functions) {
            if(!f.m_external) {
                allExt = false;
                break;
            }
        }
        if(allExt) continue;
        auto ssaOutputPath = outputPath / (file.m_path.stem().string() + ".ssa");
        std::ofstream fs(ssaOutputPath);
        SSA::Printer::print(fs, file);
        fs.close();

        File irFile = IRBuilderSSA::build(file, ctx);
        auto irOutputPath = outputPath / (file.m_path.stem().string() + ".ir");
        fs = std::ofstream(irOutputPath);
        IRPrinter::print(fs, irFile);
        fs.close();

        CodeGen::x86_64::CodeGenerator generator;
        CodeGen::x86_64::File asmFile = generator.generate(irFile, ctx);

        auto codegenOutputPath = outputPath / (irFile.m_path.stem().string() + ".asm");
        fs = std::ofstream(codegenOutputPath);
        CodeGen::x86_64::Emitter::emit(asmFile, irFile, fs, ctx);
        fs.close();
    }

    if(assemble) {
        for(SSA::File& file : ssaFiles) {
            bool allExt = true;
            for(auto& f : file.m_functions) {
                if(!f.m_external) {
                    allExt = false;
                    break;
                }
            }
            if(allExt) continue;
            std::filesystem::path codegenOutputPath = outputPath / (file.m_path.stem().string() + ".asm");
            std::filesystem::path assemblerOutputPath = outputPath / (file.m_path.stem().string() + ".o");
            std::string cmd = "as --64 -g -o \"" + assemblerOutputPath.string() + "\" \"" + codegenOutputPath.string() + "\"";
            spdlog::info("Assembling {} with command: {}", file.m_path.string(), cmd);
            int result = std::system((cmd).c_str());
            if(result != 0) {
                spdlog::error("Assembler failed with exit code {}", result);
                return 1;
            }
        }
    }

    if(link) {
        const char* stdPath = std::getenv("BRAW_STDLIB");
        if(!stdPath) {
            spdlog::error("Environment variable BRAW_STDLIB is not set");
            return 1;
        }
        std::string cmd = "gcc " + (outputPath / ("*.o")).string() + " " + (std::filesystem::path(stdPath) / "impl" / "*.o").string() + " -no-pie -m64";
        spdlog::info("Linking with command: {}", cmd);
        int result = std::system((cmd).c_str());
        if(result != 0) {
            spdlog::error("Linker failed with exit code {}", result);
            return 1;
        }
    }

    return 0;
}