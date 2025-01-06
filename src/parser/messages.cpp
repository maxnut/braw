#include "messages.hpp"
#include "parser/nodes/evaluatable.hpp"

#include <spdlog/spdlog.h>

std::string Message::where() {
    return fmt::format("{}:{}:{}:", std::filesystem::absolute(m_file).string(), m_cursor->get().value().m_line, m_cursor->value().m_column);
}

void Message::unexpectedToken(const Token& token) {
    spdlog::error("{} Unexpected token {} \"{}\"", where(), Token::typeString(token.m_type), token.m_value);
}

void Message::expectedTokenType(const std::string& type, const std::string& expected) {
    spdlog::error("{} Expected {}, got {}", where(), expected, type);
}

void Message::unknownType(const std::string& type) {
    spdlog::error("{} Unknown type {}", where(), type);
}

void Message::mismatchedTypes(const std::string& type1, const std::string& type2) {
    spdlog::error("{} Mismatched types {} and {}", where(), type1, type2);
}

void Message::unknownFunction(const std::string& name, const std::vector<std::unique_ptr<EvaluatableNode>> &parameters) {
    std::string funcString = name + "(";
    for(int i = 0; i < parameters.size(); i++) {
        funcString += parameters[i]->m_type.m_name;
        if(i < parameters.size() - 1)
            funcString += ", ";
    }
    funcString += ")";

    spdlog::error("{} Unknown function {}", where(), funcString);
}