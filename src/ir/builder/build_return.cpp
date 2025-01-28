#include "ir/instruction.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/return.hpp"

void IRBuilder::build(const AST::ReturnNode* node, BrawContext& context, IRFunctionContext& ictx) {
    if(node->m_value) {
        Operand op = buildExpression(node->m_value.get(), context, ictx);
        moveToRegister(ictx.m_returnRegister->m_id, op, context, ictx);
    }

    ictx.m_instructions.push_back(std::make_unique<Instruction>(Instruction::Return));
}