#include "parser/parser.hpp"
#include "../binary_operator.hpp"

Result<std::unique_ptr<AST::BinaryOperatorNode>> Parser::parseAssignment(TokenCursor& cursor) {
    std::unique_ptr<AST::BinaryOperatorNode> assignment = std::make_unique<AST::BinaryOperatorNode>();
    assignment->m_operator = "=";

    assignment->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    auto leftOpt = parseExpression(cursor);
    if(!leftOpt)
        return std::unexpected{leftOpt.error()};
    assignment->m_left = std::move(leftOpt.value());

    if(!expectTokenType(cursor.get().next().value(), Token::ASSIGNMENT))
        return unexpectedTokenExpectedType(cursor.value(), Token::ASSIGNMENT);

    auto rightOpt = parseExpression(cursor);
    if(!rightOpt)
        return std::unexpected{rightOpt.error()};
    assignment->m_right = std::move(rightOpt.value());

    assignment->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};

    return assignment;
}