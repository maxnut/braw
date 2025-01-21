#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantic-analyzer/semantic_analyzer.hpp"
#include "execution-tree/builder/et_builder.hpp"
#include "interpreter/interpreter.hpp"
#include "ir/builder/ir_builder.hpp"
#include "ir/printer/ir_printer.hpp"

#include <spdlog/spdlog.h>
#include <args/args.hxx>

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

    if(!ctx.m_functionTable.contains("main")) {
        spdlog::error("No entrypoint found");
        return 1;
    }

    // Stack stack;
    // Interpreter().invokeFunction(ctx.m_functionTable.at("main").at(0).get(), stack, nullptr, 0);

    auto res = IRBuilder::build(ast.value().get(), ctx);
    IRPrinter::print(std::cout, res.at(0));

    return 0;
}