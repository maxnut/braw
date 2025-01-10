#include "parser/parser.hpp"
#include "../binary_operator.hpp"

std::unique_ptr<EvaluatableNode> Parser::parseExpression(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx, int minPrecedence) {
    std::unique_ptr<EvaluatableNode> left = parseOperand(file, cursor, ctx);
    if(!left)
        return nullptr;

    while(cursor.hasNext()) {
        if(cursor.get().value().m_type != Token::OPERATOR)
            break;

        Token opToken = cursor.get().next().value();
        int precedence = Rules::s_operatorPrecedence[opToken.m_value];

        if(precedence < minPrecedence) {
            cursor.prev();
            break;
        }

        std::unique_ptr<EvaluatableNode> right = parseExpression(file, cursor, ctx, precedence + 1);
        if(!right)
            return nullptr;

        if(left->m_type != right->m_type) {
            m_message.mismatchedTypes(left->m_type.m_name, right->m_type.m_name);
            return nullptr;
        }

        if(!left->m_type.m_operators.contains(opToken.m_value)) {
            m_message.unknownOperator(opToken.m_value, left->m_type.m_name);
            return nullptr;
        }

        std::unique_ptr<BinaryOperatorNode> op = std::make_unique<BinaryOperatorNode>();
        op->m_function = left->m_type.m_operators.at(opToken.m_value).m_function;
        op->m_type = file->getTypeInfo(left->m_type.m_operators.at(opToken.m_value).m_returnType).value();
        op->m_left = std::move(left);
        op->m_right = std::move(right);
        op->m_size = op->m_left->m_size;
        op->m_memoryType = PRVALUE;

        left = std::move(op);
    }

    return left;
}