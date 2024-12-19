#pragma once

#include "nodes/file.hpp"
#include "nodes/function_definition.hpp"
#include "nodes/scope.hpp"
#include "../token.hpp"
#include "../cursor.hpp"

class Parser {
public:
    std::unique_ptr<FileNode> parseFile(std::vector<Token>& tokens);

    bool expectTokenType(const Token& token, Token::Type type);
};