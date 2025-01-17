#include "parser/parser.hpp"
#include "../unary_operator.hpp"

Result<std::unique_ptr<AST::UnaryOperatorNode>> Parser::parseDotArrow(TokenCursor& cursor, std::unique_ptr<AST::Node> left) {
    if(!expectTokenType(cursor.get().value(), Token::OPERATOR))
        return unexpectedTokenExpectedType(cursor.value(), Token::OPERATOR);

    std::unique_ptr<AST::UnaryOperatorNode> dotArrow = std::make_unique<AST::UnaryOperatorNode>();
    dotArrow->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    dotArrow->m_operator = cursor.get().next().value().m_value;
    dotArrow->m_data = cursor.get().next().value().m_value;
    dotArrow->m_operand = std::move(left);
    dotArrow->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};

    return dotArrow;
}