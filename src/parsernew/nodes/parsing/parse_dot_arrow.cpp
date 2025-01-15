#include "parsernew/parser.hpp"
#include "../unary_operator.hpp"

Result<std::shared_ptr<AST::UnaryOperatorNode>> Parser::parseDotArrow(TokenCursor& cursor, std::shared_ptr<AST::Node> left) {
    if(!expectTokenType(cursor.get().value(), Token::OPERATOR))
        return unexpectedTokenExpectedType(cursor.value(), Token::OPERATOR);

    std::shared_ptr<AST::UnaryOperatorNode> dotArrow = std::make_shared<AST::UnaryOperatorNode>();
    dotArrow->m_operator = cursor.get().next().value().m_value;
    dotArrow->m_data = cursor.get().next().value().m_value;
    dotArrow->m_operand = std::move(left);

    return dotArrow;
}