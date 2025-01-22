#include "ir_builder.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "parser/nodes/literal.hpp"
#include "parser/nodes/variable_access.hpp"

Operator IRBuilder::buildExpression(const AST::Node* node, BrawContext& context, Instructions& is) {
    switch(node->m_type) {
        case AST::Node::BinaryOperator:
            return buildBinaryOperator(static_cast<const AST::BinaryOperatorNode*>(node), context, is);
        case AST::Node::Literal:
            return Value(static_cast<const AST::LiteralNode*>(node)->m_value);
        case AST::Node::VariableAccess:
            return Register("%" + static_cast<const AST::VariableAccessNode*>(node)->m_name.m_name);
        default:
            break;
    }

    return {};
}