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

void Message::expectedType(const std::string& type, const std::string& expected) {
    spdlog::error("{} Expected type {}, got {}", where(), expected, type);
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

void Message::unknownVariable(const std::string& type) {
    spdlog::error("{} Unknown variable {}", where(), type);
}

void Message::unknownOperator(const std::string& op, const std::string& type) {
    spdlog::error("{} Unknown operator {} {} {}", where(), type, op, type);
}

void Message::unexpectedValueCategories(ValueType type, std::vector<ValueType> expected) {
    std::string expectedValues = "";
    for(int i = 0; i < expected.size(); i++) {
        expectedValues += valueTypeString(expected[i]);
        if(i < expected.size() - 1)
            expectedValues += ", ";
    }

    spdlog::error("{} Expected value of type {}, got {}", where(), expectedValues, valueTypeString(type));
}

void Message::bindFail(const std::string& name) {
    spdlog::error("{} Failed to bind {}", where(), name);
}

void Message::bindFunctionFail(const std::string& name, const std::string& function) {
    spdlog::error("{} Failed to bind function {} to {}", where(), function, name);
}