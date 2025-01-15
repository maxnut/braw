#include "parser.hpp"
#include "nodes/file.hpp"

#include <spdlog/fmt/fmt.h>

Result<std::unique_ptr<AST::FileNode>> Parser::parse(std::vector<Token> tokens) {
    TokenCursor cursor(tokens.begin(), tokens.end() - 1);

    return parseFile(cursor);
}

std::unexpected<ParseError> Parser::unexpectedToken(Token& token) {
    ParseError error {
        fmt::format("Unexpected token {} \"{}\"", Token::typeString(token.m_type), token.m_value),
        token.m_line,
        token.m_column
    };
    return std::unexpected{error};
}

std::unexpected<ParseError> Parser::unexpectedTokenExpectedType(Token& token, Token::Type expectedType) {
    ParseError error {
        fmt::format("Unexpected token {} \"{}\" expected type {}", Token::typeString(token.m_type), token.m_value, Token::typeString(expectedType)),
        token.m_line,
        token.m_column
    };
    return std::unexpected{error};
}