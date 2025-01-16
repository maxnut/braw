#include "et_builder.hpp"
#include "parser/nodes/function_call.hpp"
#include "parser/nodes/while.hpp"
#include "parser/nodes/if.hpp"
#include "parser/nodes/return.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "execution-tree/nodes/function_instruction.hpp"

std::unique_ptr<FunctionInstructionNode> ETBuilder::buildInstruction(const AST::Node* node, BrawContext& context) {
    switch(node->m_type) {
        case AST::Node::Type::FunctionCall:
            return buildFunctionCall(static_cast<const AST::FunctionCallNode*>(node), context);
        case AST::Node::Type::VariableDeclaration:
            return buildVariableDeclaration(static_cast<const AST::VariableDeclarationNode*>(node), context);
        case AST::Node::Type::While:
            return buildWhile(static_cast<const AST::WhileNode*>(node), context);
        case AST::Node::Type::If:
            return buildIf(static_cast<const AST::IfNode*>(node), context);
        case AST::Node::Type::Return:
            return buildReturn(static_cast<const AST::ReturnNode*>(node), context);
        case AST::Node::Type::BinaryOperator:
            if(static_cast<const AST::BinaryOperatorNode*>(node)->m_operator == "=")
                return buildAssignment(static_cast<const AST::BinaryOperatorNode*>(node), context);
            break;
        default:
            break;
    }

    return buildEvaluatable(node, context);
}