#include "parser/parser.hpp"
#include "../if.hpp"

Result<std::unique_ptr<AST::IfNode>> Parser::parseIf(TokenCursor& cursor) {
    if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
        return unexpectedTokenExpectedType(cursor.value(), Token::KEYWORD);

    if(!!expectTokenValue(cursor.get().value(), "if"))
        return unexpectedTokenExpectedValue(cursor.value(), "if");

    std::unique_ptr<AST::IfNode> ifNode = std::make_unique<AST::IfNode>();
    ifNode->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    
    cursor.next();

    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_PAREN);

    auto conditionOpt = parseExpression(cursor);
    if(!conditionOpt)
        return std::unexpected{conditionOpt.error()};
    ifNode->m_condition = std::move(conditionOpt.value());

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN);

    auto scopeOpt = parseScope(cursor);
    if(!scopeOpt)
        return std::unexpected{scopeOpt.error()};
    ifNode->m_then = std::move(scopeOpt.value());
    
    ifNode->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};

    return ifNode;
}