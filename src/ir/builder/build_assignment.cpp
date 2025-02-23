#include "ir/instructions/basic.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "parser/nodes/variable_access.hpp"
#include <memory>

void IRBuilder::buildAssignment(const AST::BinaryOperatorNode* node, BrawContext& context, IRFunctionContext& ictx) {
    Operand right = buildExpression(node->m_right.get(), context, ictx);
    Operand left = buildExpression(node->m_left.get(), context, ictx);

    if(left.index() == 1)
        moveToRegister("%" + std::get<std::shared_ptr<Register>>(left)->m_id, right, context, ictx);
    else
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Move, left, right));
}