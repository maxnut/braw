#include "et_builder.hpp"
#include "parser/nodes/literal.hpp"
#include "parser/nodes/variable_access.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "parser/nodes/function_call.hpp"
#include "execution-tree/nodes/evaluatable.hpp"

std::unique_ptr<EvaluatableNode> ETBuilder::buildEvaluatable(const AST::Node* node, BrawContext& context) {
    switch(node->m_type) {
        case AST::Node::Type::Literal:
            return buildLiteral(static_cast<const AST::LiteralNode*>(node), context);
        case AST::Node::Type::VariableAccess:
            return buildVariableAccess(static_cast<const AST::VariableAccessNode*>(node), context);
        case AST::Node::Type::UnaryOperator:
            return buildCast(static_cast<const AST::UnaryOperatorNode*>(node), context);
        case AST::Node::Type::BinaryOperator:
            return buildBinaryOperator(static_cast<const AST::BinaryOperatorNode*>(node), context);
        case AST::Node::Type::FunctionCall:
            return buildFunctionCall(static_cast<const AST::FunctionCallNode*>(node), context);
        default:
            break;
    }

    return nullptr;
}