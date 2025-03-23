#include "parser/parser.hpp"
#include "../while.hpp"

Result<std::shared_ptr<AST::WhileNode>> Parser::parseWhile(TokenCursor& cursor, const std::filesystem::path& path, std::shared_ptr<AST::FileNode> file) {
    if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
        return unexpectedTokenExpectedType(cursor.value(), Token::KEYWORD, path);

    if(!expectTokenValue(cursor.get().value(), "while"))
        return unexpectedTokenExpectedValue(cursor.value(), "while", path);

    std::shared_ptr<AST::WhileNode> whileNode = std::make_shared<AST::WhileNode>();
    whileNode->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};

    cursor.next();

    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_PAREN, path);

    auto conditionOpt = parseExpression(cursor, path, file);
    if(!conditionOpt)
        return std::unexpected{conditionOpt.error()};
    whileNode->m_condition = std::move(conditionOpt.value());

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN, path);

    auto scopeOpt = parseScope(cursor, path, file);
    if(!scopeOpt)
        return std::unexpected{scopeOpt.error()};
    whileNode->m_then = std::move(scopeOpt.value());
    whileNode->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};

    return whileNode;
}