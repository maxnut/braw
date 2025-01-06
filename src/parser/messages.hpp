#pragma once

#include "token.hpp"
#include "cursor.hpp"

#include <filesystem>

class EvaluatableNode;

class Message {
public:
    Message() = default;
    Message(std::filesystem::path file, Cursor<std::vector<Token>::iterator, Token>* cursor) : m_file(file), m_cursor(cursor) {}

    std::string where();
    void unexpectedToken(const Token& token);
    void expectedTokenType(const std::string& type, const std::string& expected);
    void unknownType(const std::string& type);
    void mismatchedTypes(const std::string& type1, const std::string& type2);
    void unknownFunction(const std::string& name, const std::vector<std::unique_ptr<EvaluatableNode>> &parameters);

private:
    std::filesystem::path m_file;
    Cursor<std::vector<Token>::iterator, Token>* m_cursor;
};