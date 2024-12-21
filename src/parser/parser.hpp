#pragma once

#include "nodes/file.hpp"
#include "nodes/function_definition.hpp"
#include "nodes/scope.hpp"
#include "rules.hpp"

#include <deque>

struct ScopeInfo {
    TypeInfo m_type;
    size_t m_stackIndex;
    size_t m_arraySize;
};

struct ParserFunctionContext {
    std::deque<std::unordered_map<std::string, ScopeInfo>> m_scopeTables;
    size_t m_currentStackSize = 0;
    size_t m_maxStackSize = 0;

    std::optional<ScopeInfo> get(const std::string& name) const;
    bool isDefined(const std::string& name) const;

    void changeStackSize(int amt);
    void setStackSize(size_t amt);
};

class Parser {
public:
    std::shared_ptr<FileNode> parseFile(std::vector<Token>& tokens);
    std::shared_ptr<FunctionDefinitionNode> parseFunctionDefinition(std::shared_ptr<FileNode> file, TokenCursor& cursor);
    std::unique_ptr<ScopeNode> parseScope(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx);
    std::unique_ptr<FunctionInstructionNode> parseInstruction(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx);

    bool expectTokenType(const Token& token, Token::Type type);
};