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
struct MacroNode;
struct MacroParameterNode;
}

struct ParseError {
    std::string m_message;
    std::filesystem::path m_path;
    int m_line;
    int m_column;
};

struct ParserContext {
    const std::filesystem::path& m_path;
    std::shared_ptr<AST::FileNode> m_file;
    std::shared_ptr<AST::MacroNode> m_currentMacro = nullptr;
};

template <typename T>
using Result = std::expected<T, ParseError>;

class Parser {
public:
    static Result<std::shared_ptr<AST::FileNode>> parse(std::vector<Token> tokens, std::filesystem::path path);

private:
    static Result<std::shared_ptr<AST::FileNode>> parseFile(TokenCursor& cursor, std::filesystem::path path);
    static Result<std::shared_ptr<AST::FunctionDefinitionNode>> parseFunctionDefinition(TokenCursor& cursor, ParserContext& ctx);
    static Result<std::shared_ptr<AST::ScopeNode>> parseScope(TokenCursor& cursor, ParserContext& ctx, bool allowOneLine = true);
    static Result<std::shared_ptr<AST::Node>> parseInstruction(TokenCursor& cursor, ParserContext& ctx);
    static Result<std::shared_ptr<AST::VariableDeclarationNode>> parseVariableDeclaration(TokenCursor& cursor, ParserContext& ctx, bool omitLet = false);
    static Result<std::shared_ptr<AST::Node>> parseExpression(TokenCursor& cursor, ParserContext& ctx, int minPrecedence = 0);
    static Result<std::shared_ptr<AST::Node>> parseOperand(TokenCursor& cursor, ParserContext& ctx);
    static Result<std::shared_ptr<AST::Node>> parsePrimary(TokenCursor& cursor, ParserContext& ctx);
    static Result<std::shared_ptr<AST::LiteralNode>> parseLiteral(TokenCursor& cursor, ParserContext& ctx);
    static Result<std::shared_ptr<AST::FunctionCallNode>> parseFunctionCall(TokenCursor& cursor, ParserContext& ctx);
    static Result<std::shared_ptr<AST::VariableAccessNode>> parseVariableAccess(TokenCursor& cursor, ParserContext& ctx);
    static Result<std::shared_ptr<AST::UnaryOperatorNode>> parseDotArrow(TokenCursor& cursor, std::shared_ptr<AST::Node> left, ParserContext& ctx);
    static Result<std::shared_ptr<AST::UnaryOperatorNode>> parseCast(TokenCursor& cursor, ParserContext& ctx);
    static Result<std::shared_ptr<AST::BinaryOperatorNode>> parseAssignment(TokenCursor& cursor, ParserContext& ctx);
    static Result<std::shared_ptr<AST::ReturnNode>> parseReturn(TokenCursor& cursor, ParserContext& ctx);
    static Result<std::shared_ptr<AST::IfNode>> parseIf(TokenCursor& cursor, ParserContext& ctx);
    static Result<std::shared_ptr<AST::WhileNode>> parseWhile(TokenCursor& cursor, ParserContext& ctx);
    static Result<std::shared_ptr<AST::ForNode>> parseFor(TokenCursor& cursor, ParserContext& ctx);
    static Result<std::shared_ptr<AST::StructNode>> parseStructDefinition(TokenCursor& cursor, ParserContext& ctx);
    static Result<std::shared_ptr<AST::FileNode>> parseImport(TokenCursor& cursor, ParserContext& ctx);
    static Result<std::shared_ptr<AST::MacroNode>> parseMacro(TokenCursor& cursor, ParserContext& ctx);
    static Result<std::shared_ptr<AST::MacroParameterNode>> parseMacroParameter(TokenCursor& cursor, ParserContext& ctx);
    static Result<std::shared_ptr<AST::Node>> parseMacroCall(TokenCursor& cursor, ParserContext& ctx);
    static Result<AST::FunctionSignature> parseFunctionSignature(TokenCursor& cursor, ParserContext& ctx);
    static Result<Identifier> parseTypename(TokenCursor& cursor, ParserContext& ctx);

    static std::unexpected<ParseError> unexpectedToken(Token& token, const std::filesystem::path& path);
    static std::unexpected<ParseError> unexpectedTokenExpectedType(Token& token, Token::Type expectedType, const std::filesystem::path& path);
    static std::unexpected<ParseError> unexpectedTokenExpectedTypes(Token& token, std::vector<Token::Type> expectedTypes, const std::filesystem::path& path);
    static std::unexpected<ParseError> unexpectedTokenExpectedValue(Token& token, const std::string& expectedValue, const std::filesystem::path& path);
    static std::unexpected<ParseError> notMacro(Token& token, const std::filesystem::path& path);
    static std::unexpected<ParseError> unknownMacro(Token& token, const std::filesystem::path& path);
    static std::unexpected<ParseError> unknownMacroParameter(Token& token, const std::filesystem::path& path);

    static bool expectTokenType(const Token& token, Token::Type type) { return token.m_type == type; }
    static bool expectTokenTypes(const Token& token, std::vector<Token::Type> types) { return std::find(types.begin(), types.end(), token.m_type) != types.end(); }
    static bool expectTokenValue(const Token& token, const std::string& value) { return token.m_value == value; }
    
};