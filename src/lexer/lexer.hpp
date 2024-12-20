#pragma once

#include "../token.hpp"
#include "../cursor.hpp"

#include <filesystem>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <optional>

class Lexer {
public:
    Lexer(std::filesystem::path path);
    std::optional<std::vector<Token>> tokenize();

private:
    Token parseNumber(Cursor<std::string::iterator, char>& cursor, int lineNumber);
    Token parseAlphanumeric(Cursor<std::string::iterator, char>& cursor, int lineNumber);
    Token parseString(Cursor<std::string::iterator, char>& cursor, int lineNumber);

    Token tryParseSingleToken(Cursor<std::string::iterator, char> cursor, int lineNumber);

private:
    std::filesystem::path m_path;
};