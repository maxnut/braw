#include "messages.hpp"

#include <spdlog/spdlog.h>

std::string Message::where() {
    return fmt::format("{} {},{}: ", m_file.filename().string(), m_cursor->get().value().m_line, m_cursor->value().m_column);
}

void Message::unexpectedToken(const Token& token) {
    spdlog::error("{} Unexpected token {} \"{}\"", where(), Token::typeString(token.m_type), token.m_value);
}

void Message::expectedTokenType(const std::string& type, const std::string& expected) {
    spdlog::error("{} Expected {}, got {}", where(), type, expected);
}

void Message::unknownType(const std::string& type) {
    spdlog::error("{} Unknown type {}", where(), type);
}

void Message::mismatchedTypes(const std::string& type1, const std::string& type2) {
    spdlog::error("{} Mismatched types {} and {}", where(), type1, type2);
}