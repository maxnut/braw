#include "codegen/x86-64/code_generator.hpp"
#include "codegen/x86-64/emitter.hpp"
#include "codegen/x86-64/file.hpp"
#include "ir/file.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantic-analyzer/semantic_analyzer.hpp"
#include "ir/builder/ir_builder.hpp"
#include "ir/printer/ir_printer.hpp"
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
    args::ValueFlag<std::string> assembler(parser, "assembler", "Assembler to use (nasm or gas)", {'a', "assembler"}, "gas");
    args::Flag assemble(parser, "assemble", "Assemble the output file", {"assemble"});
    args::Flag link(parser, "link", "Link the output file", {'l', "link"});

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

    std::string assemblerChoice = assembler.Get();
    if (assemblerChoice != "nasm" && assemblerChoice != "gas") {
        spdlog::error("Invalid assembler choice. Use 'nasm' or 'gas'.");
        return 1;
    }
    auto tokens = Lexer::tokenize(filepath);
    if (!tokens) {
        spdlog::error("Failed to tokenize file");
        return 1;
    }

    auto ast = Parser::parse(tokens.value(), filepath);
    if (!ast) {
        spdlog::error("{}:{} {}", ast.error().m_line, ast.error().m_column, ast.error().m_message);
        return 1;
    }

    auto ctxOr = SemanticAnalyzer::analyze(ast.value().get());
    if (!ctxOr) {
        spdlog::error("{}:{} {}", ctxOr.error().m_rangeBegin.first, ctxOr.error().m_rangeBegin.second, ctxOr.error().m_message);
        return 1;
    }

    BrawContext ctx = ctxOr.value();

    if(assemblerChoice == "nasm")
        ctx.m_assembler = NASM;
    else if(assemblerChoice == "gas")
        ctx.m_assembler = GAS;
    
    std::vector<File> res = IRBuilder::build(ast.value().get(), ctx);

    for(File& file : res) {
        bool allExt = true;
        for(auto& f : file.m_functions) {
            if(!f.m_external) {
                allExt = false;
                break;
            }
        }
        if(allExt) continue;
        auto irOutputPath = outputPath / (file.m_path.stem().string() + ".ir");
        std::ofstream fs(irOutputPath);
        IRPrinter::print(fs, file);
        fs.close();

        CodeGen::x86_64::CodeGenerator generator;
        CodeGen::x86_64::File asmFile = generator.generate(file, ctx);

        auto codegenOutputPath = outputPath / (file.m_path.stem().string() + ".asm");
        fs = std::ofstream(codegenOutputPath);
        CodeGen::x86_64::Emitter::emit(asmFile, file, fs, ctx);
        fs.close();
    }

    if(assemble) {
        for(File& file : res) {
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
            std::string prefix = ctx.m_assembler == NASM ? "nasm -f elf64" : "as --64 -g";
            std::string cmd = prefix + " -o \"" + assemblerOutputPath.string() + "\" \"" + codegenOutputPath.string() + "\"";
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