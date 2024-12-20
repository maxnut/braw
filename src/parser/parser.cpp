#include "parser.hpp"

#include <spdlog/spdlog.h>

bool Parser::expectTokenType(const Token& token, Token::Type type) {
    if(token.m_type != type)
        spdlog::error("Expected {}, got {}", Token::typeString(type), Token::typeString(token.m_type));

    return token.m_type == type;
}