#pragma once

#include "nodes/function_definition.hpp"
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
struct CastNode;
struct IfNode;
struct WhileNode;
struct LiteralNode;
struct ReturnNode;
struct ImportNode;
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
    Result<std::unique_ptr<AST::FileNode>> parse(std::vector<Token> tokens);

public:
    Result<std::unique_ptr<AST::FileNode>> parseFile(TokenCursor& cursor);
    Result<std::unique_ptr<AST::FunctionDefinitionNode>> parseFunctionDefinition(TokenCursor& cursor);
    Result<std::unique_ptr<AST::ScopeNode>> parseScope(TokenCursor& cursor);
    Result<std::unique_ptr<AST::Node>> parseInstruction(TokenCursor& cursor);
    Result<std::unique_ptr<AST::VariableDeclarationNode>> parseVariableDeclaration(TokenCursor& cursor);
    Result<std::unique_ptr<AST::Node>> parseExpression(TokenCursor& cursor, int minPrecedence = 0);
    Result<std::unique_ptr<AST::Node>> parseOperand(TokenCursor& cursor);
    Result<std::unique_ptr<AST::Node>> parsePrimary(TokenCursor& cursor);
    Result<std::unique_ptr<AST::LiteralNode>> parseLiteral(TokenCursor& cursor);
    Result<std::unique_ptr<AST::FunctionCallNode>> parseFunctionCall(TokenCursor& cursor);
    Result<std::unique_ptr<AST::VariableAccessNode>> parseVariableAccess(TokenCursor& cursor);
    Result<std::unique_ptr<AST::UnaryOperatorNode>> parseDot(TokenCursor& cursor, std::unique_ptr<AST::Node> left);
    Result<std::unique_ptr<AST::UnaryOperatorNode>> parseArrow(TokenCursor& cursor, std::unique_ptr<AST::Node> left);
    Result<std::unique_ptr<AST::CastNode>> parseCast(TokenCursor& cursor);
    Result<std::unique_ptr<AST::BinaryOperatorNode>> parseAssignment(TokenCursor& cursor);
    Result<std::unique_ptr<AST::ReturnNode>> parseReturn(TokenCursor& cursor);
    Result<std::unique_ptr<AST::IfNode>> parseIf(TokenCursor& cursor);
    Result<std::unique_ptr<AST::WhileNode>> parseWhile(TokenCursor& cursor);
    Result<std::unique_ptr<AST::StructNode>> parseStructDefinition(TokenCursor& cursor);
    Result<std::unique_ptr<AST::BindNode>> parseBind(TokenCursor& cursor);
    Result<std::unique_ptr<AST::ImportNode>> parseImport(TokenCursor& cursor);
    Result<AST::FunctionSignature> parseFunctionSignature(TokenCursor& cursor);
    Result<Identifier> parseTypename(TokenCursor& cursor);

    std::unexpected<ParseError> unexpectedToken(Token& token);
    std::unexpected<ParseError> unexpectedTokenExpectedType(Token& token, Token::Type expectedType);

    bool expectTokenType(const Token& token, Token::Type type) { return token.m_type == type; }
    
};