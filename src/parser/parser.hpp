#pragma once

#include "nodes/file.hpp"
#include "identifier.hpp"
#include "rules.hpp"

#include <expected>
#include <memory>

namespace AST {
struct FileNode;
struct FunctionDefinitionNode;
struct ScopeNode;
struct VariableDeclarationNode;
struct VariableAccessNode;
struct UnaryOperatorNode;
struct BinaryOperatorNode;
struct StructNode;
struct FunctionCallNode;
struct BindNode;
struct IfNode;
struct WhileNode;
struct LiteralNode;
struct ReturnNode;
struct ForNode;
}

struct ParseError {
    std::string m_message;
    std::filesystem::path m_path;
    int m_line;
    int m_column;
};

template <typename T>
using Result = std::expected<T, ParseError>;

class Parser {
public:
    static Result<std::shared_ptr<AST::FileNode>> parse(std::vector<Token> tokens, std::filesystem::path path);

private:
    static Result<std::shared_ptr<AST::FileNode>> parseFile(TokenCursor& cursor, std::filesystem::path path);
    static Result<std::unique_ptr<AST::FunctionDefinitionNode>> parseFunctionDefinition(TokenCursor& cursor, const std::filesystem::path& path);
    static Result<std::unique_ptr<AST::ScopeNode>> parseScope(TokenCursor& cursor, const std::filesystem::path& path, bool allowOneLine = true);
    static Result<std::unique_ptr<AST::Node>> parseInstruction(TokenCursor& cursor, const std::filesystem::path& path);
    static Result<std::unique_ptr<AST::VariableDeclarationNode>> parseVariableDeclaration(TokenCursor& cursor, const std::filesystem::path& path, bool omitLet = false);
    static Result<std::unique_ptr<AST::Node>> parseExpression(TokenCursor& cursor, const std::filesystem::path& path, int minPrecedence = 0);
    static Result<std::unique_ptr<AST::Node>> parseOperand(TokenCursor& cursor, const std::filesystem::path& path);
    static Result<std::unique_ptr<AST::Node>> parsePrimary(TokenCursor& cursor, const std::filesystem::path& path);
    static Result<std::unique_ptr<AST::LiteralNode>> parseLiteral(TokenCursor& cursor, const std::filesystem::path& path);
    static Result<std::unique_ptr<AST::FunctionCallNode>> parseFunctionCall(TokenCursor& cursor, const std::filesystem::path& path);
    static Result<std::unique_ptr<AST::VariableAccessNode>> parseVariableAccess(TokenCursor& cursor, const std::filesystem::path& path);
    static Result<std::unique_ptr<AST::UnaryOperatorNode>> parseDotArrow(TokenCursor& cursor, std::unique_ptr<AST::Node> left, const std::filesystem::path& path);
    static Result<std::unique_ptr<AST::UnaryOperatorNode>> parseCast(TokenCursor& cursor, const std::filesystem::path& path);
    static Result<std::unique_ptr<AST::BinaryOperatorNode>> parseAssignment(TokenCursor& cursor, const std::filesystem::path& path);
    static Result<std::unique_ptr<AST::ReturnNode>> parseReturn(TokenCursor& cursor, const std::filesystem::path& path);
    static Result<std::unique_ptr<AST::IfNode>> parseIf(TokenCursor& cursor, const std::filesystem::path& path);
    static Result<std::unique_ptr<AST::WhileNode>> parseWhile(TokenCursor& cursor, const std::filesystem::path& path);
    static Result<std::unique_ptr<AST::ForNode>> parseFor(TokenCursor& cursor, const std::filesystem::path& path);
    static Result<std::unique_ptr<AST::StructNode>> parseStructDefinition(TokenCursor& cursor, const std::filesystem::path& path);
    static Result<std::shared_ptr<AST::FileNode>> parseImport(TokenCursor& cursor, const std::filesystem::path& path);
    static Result<AST::FunctionSignature> parseFunctionSignature(TokenCursor& cursor, const std::filesystem::path& path);
    static Result<Identifier> parseTypename(TokenCursor& cursor, const std::filesystem::path& path);

    static std::unexpected<ParseError> unexpectedToken(Token& token, const std::filesystem::path& path);
    static std::unexpected<ParseError> unexpectedTokenExpectedType(Token& token, Token::Type expectedType, const std::filesystem::path& path);
    static std::unexpected<ParseError> unexpectedTokenExpectedTypes(Token& token, std::vector<Token::Type> expectedTypes, const std::filesystem::path& path);
    static std::unexpected<ParseError> unexpectedTokenExpectedValue(Token& token, const std::string& expectedValue, const std::filesystem::path& path);

    static bool expectTokenType(const Token& token, Token::Type type) { return token.m_type == type; }
    static bool expectTokenTypes(const Token& token, std::vector<Token::Type> types) { return std::find(types.begin(), types.end(), token.m_type) != types.end(); }
    static bool expectTokenValue(const Token& token, const std::string& value) { return token.m_value == value; }
    
};