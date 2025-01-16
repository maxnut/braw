#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "ast-printer/ast_printer.hpp"

#include <spdlog/spdlog.h>
#include <args/args.hxx>

#include <iostream>

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

    ASTPrinter::print(ast.value().get());

    return 0;
}