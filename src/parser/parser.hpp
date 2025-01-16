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
}

struct ParseError {
    std::string m_message;
    int m_line;
    int m_column;
};

template <typename T>
using Result = std::expected<T, ParseError>;

class Parser {
public:
    static Result<std::unique_ptr<AST::FileNode>> parse(std::vector<Token> tokens);

private:
    static Result<std::unique_ptr<AST::FileNode>> parseFile(TokenCursor& cursor);
    static Result<std::unique_ptr<AST::FunctionDefinitionNode>> parseFunctionDefinition(TokenCursor& cursor);
    static Result<std::unique_ptr<AST::ScopeNode>> parseScope(TokenCursor& cursor);
    static Result<std::unique_ptr<AST::Node>> parseInstruction(TokenCursor& cursor);
    static Result<std::unique_ptr<AST::Node>> parseVariableDeclaration(TokenCursor& cursor);
    static Result<std::unique_ptr<AST::Node>> parseExpression(TokenCursor& cursor, int minPrecedence = 0);
    static Result<std::unique_ptr<AST::Node>> parseOperand(TokenCursor& cursor);
    static Result<std::unique_ptr<AST::Node>> parsePrimary(TokenCursor& cursor);
    static Result<std::unique_ptr<AST::LiteralNode>> parseLiteral(TokenCursor& cursor);
    static Result<std::unique_ptr<AST::FunctionCallNode>> parseFunctionCall(TokenCursor& cursor);
    static Result<std::unique_ptr<AST::VariableAccessNode>> parseVariableAccess(TokenCursor& cursor);
    static Result<std::unique_ptr<AST::UnaryOperatorNode>> parseDotArrow(TokenCursor& cursor, std::unique_ptr<AST::Node> left);
    static Result<std::unique_ptr<AST::UnaryOperatorNode>> parseCast(TokenCursor& cursor);
    static Result<std::unique_ptr<AST::BinaryOperatorNode>> parseAssignment(TokenCursor& cursor);
    static Result<std::unique_ptr<AST::ReturnNode>> parseReturn(TokenCursor& cursor);
    static Result<std::unique_ptr<AST::IfNode>> parseIf(TokenCursor& cursor);
    static Result<std::unique_ptr<AST::WhileNode>> parseWhile(TokenCursor& cursor);
    static Result<std::unique_ptr<AST::StructNode>> parseStructDefinition(TokenCursor& cursor);
    static Result<std::unique_ptr<AST::BindNode>> parseBind(TokenCursor& cursor);
    static Result<std::unique_ptr<AST::FileNode>> parseImport(TokenCursor& cursor);
    static Result<AST::FunctionSignature> parseFunctionSignature(TokenCursor& cursor);
    static Result<Identifier> parseTypename(TokenCursor& cursor);

    static std::unexpected<ParseError> unexpectedToken(Token& token);
    static std::unexpected<ParseError> unexpectedTokenExpectedType(Token& token, Token::Type expectedType);
    static std::unexpected<ParseError> unexpectedTokenExpectedTypes(Token& token, std::vector<Token::Type> expectedTypes);
    static std::unexpected<ParseError> unexpectedTokenExpectedValue(Token& token, const std::string& expectedValue);

    static bool expectTokenType(const Token& token, Token::Type type) { return token.m_type == type; }
    static bool expectTokenTypes(const Token& token, std::vector<Token::Type> types) { return std::find(types.begin(), types.end(), token.m_type) != types.end(); }
    static bool expectTokenValue(const Token& token, const std::string& value) { return token.m_value == value; }
    
};