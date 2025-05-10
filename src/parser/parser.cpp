#include "parser.hpp"
#include "nodes/file.hpp"

#include <spdlog/fmt/fmt.h>

Result<std::shared_ptr<AST::FileNode>> Parser::parse(std::vector<Token> tokens, std::filesystem::path path) {
    TokenCursor cursor(tokens.begin(), tokens.end() - 1);

    return parseFile(cursor, path);
}

std::unexpected<ParseError> Parser::unexpectedToken(Token& token, const std::filesystem::path& path) {
    ParseError error {
        fmt::format("Unexpected token {} \"{}\"", Token::typeString(token.m_type), token.m_value),
        path,
        token.m_line,
        token.m_column
    };
    return std::unexpected{error};
}

std::unexpected<ParseError> Parser::unexpectedTokenExpectedType(Token& token, Token::Type expectedType, const std::filesystem::path& path) {
    ParseError error {
        fmt::format("Unexpected token {} \"{}\" expected type {}", Token::typeString(token.m_type), token.m_value, Token::typeString(expectedType)),
        path,
        token.m_line,
        token.m_column
    };
    return std::unexpected{error};
}

std::unexpected<ParseError> Parser::unexpectedTokenExpectedTypes(Token& token, std::vector<Token::Type> expectedTypes, const std::filesystem::path& path) {
    std::string expectedTypesString;
    for(Token::Type type : expectedTypes) {
        expectedTypesString += Token::typeString(type) + " ";
    }
    ParseError error {
        fmt::format("Unexpected token {} \"{}\" expected types {}", Token::typeString(token.m_type), token.m_value, expectedTypesString),
        path,
        token.m_line,
        token.m_column
    };
    return std::unexpected{error};
}

std::unexpected<ParseError> Parser::unexpectedTokenExpectedValue(Token& token, const std::string& expectedValue, const std::filesystem::path& path) {
    ParseError error {
        fmt::format("Unexpected token {} \"{}\" expected value {}", Token::typeString(token.m_type), token.m_value, expectedValue),
        path,
        token.m_line,
        token.m_column
    };
    return std::unexpected{error};
}

std::unexpected<ParseError> Parser::unexpectedTokenExpectedValues(Token& token, const std::vector<std::string>& expectedValues, const std::filesystem::path& path) {
    std::string expectedValuesString;
    for(std::string value : expectedValues) {
        expectedValuesString += value + " ";
    }
    ParseError error {
        fmt::format("Unexpected token {} \"{}\" expected values {}", Token::typeString(token.m_type), token.m_value, expectedValuesString),
        path,
        token.m_line,
        token.m_column
    };
    return std::unexpected{error};
}

std::unexpected<ParseError> Parser::notMacro(Token& token, const std::filesystem::path& path) {
    ParseError error {
        fmt::format("Tried to access macro parameter {} but not currently in a macro", token.m_value),
        path,
        token.m_line,
        token.m_column
    };
    return std::unexpected{error};
}
