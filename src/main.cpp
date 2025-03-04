#include "codegen/x86-64/code_generator.hpp"
#include "codegen/x86-64/emitter.hpp"
#include "codegen/x86-64/file.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantic-analyzer/semantic_analyzer.hpp"
#include "ir/builder/ir_builder.hpp"
#include "ir/printer/ir_printer.hpp"

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
    args::ValueFlag<std::string> buildFolder(parser, "build", "Build output directory", {'b', "build"}, "build");
    args::ValueFlag<std::string> assembler(parser, "assembler", "Assembler to use (nasm or gas)", {'a', "assembler"}, "gas");

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

    std::filesystem::path filepath(inputFile.Get());
    std::filesystem::path buildPath = buildFolder ? buildFolder.Get() : "build";
    std::filesystem::create_directories(buildPath);

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

    auto ast = Parser::parse(tokens.value());
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

    std::ofstream fs(buildPath / (filepath.stem().string() + ".ir"));
    IRPrinter::print(fs, res.at(0));
    fs.close();

    CodeGen::x86_64::CodeGenerator codegen;
    CodeGen::x86_64::File file = codegen.generate(res.at(0), ctx);

    fs = std::ofstream(buildPath / (filepath.stem().string() + ".asm"));
    CodeGen::x86_64::Emitter::emit(file, res.at(0), fs, ctx);
    fs.close();

    return 0;
}