#include "ir/value.hpp"
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
        case AST::Node::VariableAccess:{
            auto var = static_cast<const AST::VariableAccessNode*>(node);
            for(int i = ictx.m_scopeDepth; i >= 0; i--) {
                if(ictx.m_registers.contains("%" + var->m_name.m_name + "_" + std::to_string(i)))
                    return ictx.m_registers["%" + var->m_name.m_name + "_" + std::to_string(i)];
            }

            return Value(-69);
        }
        case AST::Node::FunctionCall:
            return buildCall(static_cast<const AST::FunctionCallNode*>(node), context, ictx);
        default:
            break;
    }

    return {};
}