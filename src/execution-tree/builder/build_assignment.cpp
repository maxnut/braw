#include "et_builder.hpp"
#include "execution-tree/nodes/function_instruction.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "execution-tree/nodes/assignment.hpp"

std::unique_ptr<FunctionInstructionNode> ETBuilder::buildAssignment(const AST::BinaryOperatorNode* node, BrawContext& context) {
    std::unique_ptr<AssignmentNode> assignment = std::make_unique<AssignmentNode>();

    assignment->m_variable = buildEvaluatable(node->m_left.get(), context);
    assignment->m_value = buildEvaluatable(node->m_right.get(), context);

    return assignment;
}