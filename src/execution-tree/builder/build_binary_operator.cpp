#include "et_builder.hpp"
#include "parser/nodes/binary_operator.hpp"

std::unique_ptr<FunctionInstructionNode> ETBuilder::build(const AST::BinaryOperatorNode* node, BrawContext& context) {
    if(node->m_operator == "=")
        return buildAssignment(node, context);
}