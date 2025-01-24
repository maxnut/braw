#include "ir_builder.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "parser/nodes/literal.hpp"
#include "parser/nodes/variable_access.hpp"
#include "parser/nodes/function_call.hpp"

Operator IRBuilder::buildExpression(const AST::Node* node, BrawContext& context, IRFunctionContext& ictx) {
    switch(node->m_type) {
        case AST::Node::BinaryOperator:
            return buildBinaryOperator(static_cast<const AST::BinaryOperatorNode*>(node), context, ictx);
        case AST::Node::Literal:
            return Value(static_cast<const AST::LiteralNode*>(node)->m_value);
        case AST::Node::VariableAccess:
            return Register(ictx.getRegisterInfo(static_cast<const AST::VariableAccessNode*>(node)->m_name.m_name).m_name);
        case AST::Node::FunctionCall:
            return buildCall(static_cast<const AST::FunctionCallNode*>(node), context, ictx);
        default:
            break;
    }

    return {};
}