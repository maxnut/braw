#include "compiler/compiler.hpp"
#include "ir/graph-color/graph_color.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantic-analyzer/semantic_analyzer.hpp"
#include "execution-tree/builder/et_builder.hpp"
#include "interpreter/interpreter.hpp"
#include "ir/builder/ir_builder.hpp"
#include "ir/printer/ir_printer.hpp"

#include <fstream>
#include <spdlog/spdlog.h>
#include <args/args.hxx>
#include <stdio.h>

#ifdef WIN32
    #define popen _popen
    #define pclose _pclose
#endif

int main(int argc, char** argv) {
    spdlog::set_pattern("[%^%l%$] %v");
    spdlog::set_level(spdlog::level::debug);

    if(argc < 2) {
        spdlog::error("No file specified");
        return 1;
    }

    std::filesystem::path filepath(argv[1]);
    auto tokens = Lexer::tokenize(filepath);
    if(!tokens) {
        spdlog::error("Failed to tokenize file");
        return 1;
    }

    auto ast = Parser::parse(tokens.value());
    if(!ast) {
        spdlog::error("{}:{} {}", ast.error().m_line, ast.error().m_column, ast.error().m_message);
        return 1;
    }

    auto ctxOr = SemanticAnalyzer::analyze(ast.value().get());
    if(!ctxOr) {
        spdlog::error("{}:{} {}", ctxOr.error().m_rangeBegin.first, ctxOr.error().m_rangeBegin.second, ctxOr.error().m_message);
        return 1;
    }

    BrawContext ctx = ctxOr.value();

    std::shared_ptr<FileNode> file = ETBuilder::buildFile(ast.value().get(), ctx);
    if(!file) {
        spdlog::error("Failed to build execution tree");
        return 1;
    }

    auto res = IRBuilder::build(ast.value().get(), ctx);

    std::filesystem::create_directories("build");
    std::filesystem::path buildPath = std::filesystem::current_path() / "build";

    std::ofstream fs(buildPath / (filepath.stem().string() + ".ir"));
    IRPrinter::print(fs, res.at(0));
    fs.close();

    Compiler::compile(res.at(0), buildPath / (filepath.stem().string() + ".asm"));

    FILE *pipe = popen(("nasm -felf64 " + (buildPath / (filepath.stem().string() + ".asm")).string() + " -o " + (buildPath / (filepath.stem().string() + ".o")).string()).c_str(), "r");
    if (!pipe) {
        perror("popen failed");
        return 1;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        printf("%s", buffer);
    }
    
    int status = pclose(pipe);

    if(status != 0)
        spdlog::error("Assembler exited with status: {}", status);
    else
        spdlog::info("Assembler exited with status: {}", status);

    return 0;
}