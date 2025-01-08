#pragma once

#include "messages.hpp"
#include "nodes/file.hpp"
#include "nodes/function_definition.hpp"
#include "nodes/scope.hpp"
#include "nodes/evaluatable.hpp"
#include "rules.hpp"

#include <deque>

struct ScopeInfo {
    ScopeInfo(TypeInfo type, size_t stackIndex, size_t arraySize) : m_type(type), m_stackIndex(stackIndex), m_arraySize(arraySize) {}
    ScopeInfo(TypeInfo type, size_t stackIndex, int arraySize) : m_type(type), m_stackIndex(stackIndex), m_arraySize(arraySize) {}
    ScopeInfo() {}

    size_t getSize() const { return m_type.m_size * (m_arraySize == 0 ? 1 : m_arraySize); }

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
    Parser(const std::filesystem::path& path) : m_file(path) {}

    std::shared_ptr<FileNode> parse();
    std::shared_ptr<FunctionDefinitionNode> parseFunctionDefinition(std::shared_ptr<FileNode> file, TokenCursor& cursor);
    std::unique_ptr<ScopeNode> parseScope(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx);
    std::unique_ptr<FunctionInstructionNode> parseInstruction(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx);
    std::unique_ptr<FunctionInstructionNode> parseVariableDeclaration(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx);
    std::unique_ptr<EvaluatableNode> parseExpression(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx, int minPrecedence = 0);
    std::unique_ptr<EvaluatableNode> parseOperand(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx);
    std::unique_ptr<EvaluatableNode> parsePrimary(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx);
    std::unique_ptr<EvaluatableNode> parseVariable(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx);
    std::unique_ptr<EvaluatableNode> parseLiteral(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx);
    std::unique_ptr<EvaluatableNode> parseFunctionCall(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx);
    std::unique_ptr<EvaluatableNode> parseVariableAccess(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx);
    std::unique_ptr<FunctionInstructionNode> parseAssignment(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx);
    std::unique_ptr<FunctionInstructionNode> parseReturn(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx);
    std::unique_ptr<FunctionInstructionNode> parseIf(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx);

    bool expectTokenType(const Token& token, Token::Type type);
    bool expectTokenValue(const Token& token, const std::string& value);

    TypeInfo makePointer(const TypeInfo& base);
    std::string getRawType(const std::string& type);

private:
    std::shared_ptr<FileNode> parseFile(std::vector<Token>& tokens);

private:
    std::filesystem::path m_file;
    Message m_message;
};