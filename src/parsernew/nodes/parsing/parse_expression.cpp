#include "parsernew/parser.hpp"
#include "../binary_operator.hpp"

Result<std::shared_ptr<AST::Node>> Parser::parseExpression(TokenCursor& cursor, int minPrecedence) {
    auto leftOpt = parseOperand(cursor);
    if(!leftOpt)
        return std::unexpected{leftOpt.error()};

    std::shared_ptr<AST::Node> left = std::move(leftOpt.value());

    while(cursor.hasNext()) {
        if(cursor.get().value().m_type != Token::OPERATOR)
            break;

        Token opToken = cursor.get().next().value();
        int precedence = Rules::s_operatorPrecedence[opToken.m_value];

        if(precedence < minPrecedence) {
            cursor.prev();
            break;
        }

        auto rightOpt = parseExpression(cursor, precedence + 1);
        if(!rightOpt)
            return std::unexpected{rightOpt.error()};

        std::shared_ptr<AST::BinaryOperatorNode> op = std::make_shared<AST::BinaryOperatorNode>();
        op->m_operator = opToken.m_value;
        op->m_left = std::move(left);
        op->m_right = std::move(rightOpt.value());

        left = std::move(op);
    }

    return left;
}