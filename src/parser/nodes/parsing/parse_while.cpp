#include "parser/parser.hpp"
#include "../while.hpp"

Result<std::shared_ptr<AST::WhileNode>> Parser::parseWhile(TokenCursor& cursor, ParserContext& ctx) {
    if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
        return unexpectedTokenExpectedType(cursor.value(), Token::KEYWORD, ctx.m_path);

    if(!expectTokenValue(cursor.get().value(), "while"))
        return unexpectedTokenExpectedValue(cursor.value(), "while", ctx.m_path);

    std::shared_ptr<AST::WhileNode> whileNode = std::make_shared<AST::WhileNode>();
    whileNode->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};

    cursor.next();

    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_PAREN, ctx.m_path);

    auto conditionOpt = parseExpression(cursor, ctx);
    if(!conditionOpt)
        return std::unexpected{conditionOpt.error()};
    whileNode->m_condition = std::move(conditionOpt.value());

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN, ctx.m_path);

    auto scopeOpt = parseScope(cursor, ctx);
    if(!scopeOpt)
        return std::unexpected{scopeOpt.error()};
    whileNode->m_then = std::move(scopeOpt.value());
    whileNode->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};

    return whileNode;
}