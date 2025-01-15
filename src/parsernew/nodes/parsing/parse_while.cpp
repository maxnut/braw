#include "parsernew/parser.hpp"
#include "../while.hpp"

Result<std::shared_ptr<AST::WhileNode>> Parser::parseWhile(TokenCursor& cursor) {
    if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
        return unexpectedTokenExpectedType(cursor.value(), Token::KEYWORD);

    if(!expectTokenValue(cursor.get().value(), "while"))
        return unexpectedTokenExpectedValue(cursor.value(), "while");
    cursor.next();

    std::shared_ptr<AST::WhileNode> whileNode = std::make_shared<AST::WhileNode>();

    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_PAREN);

    auto conditionOpt = parseExpression(cursor);
    if(!conditionOpt)
        return std::unexpected{conditionOpt.error()};
    whileNode->m_condition = std::move(conditionOpt.value());

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN);

    auto scopeOpt = parseScope(cursor);
    if(!scopeOpt)
        return std::unexpected{scopeOpt.error()};
    whileNode->m_then = std::move(scopeOpt.value());

    return whileNode;
}