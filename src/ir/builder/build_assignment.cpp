#include "ir/instructions/binary.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "parser/nodes/variable_access.hpp"

void IRBuilder::buildAssignment(const AST::BinaryOperatorNode* node, BrawContext& context, IRFunctionContext& ictx) {
    Operator op = buildExpression(node->m_right.get(), context, ictx);

    const AST::VariableAccessNode* var = dynamic_cast<const AST::VariableAccessNode*>(node->m_left.get());
    ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, Register{"%" + var->m_name.m_name}, op));
}