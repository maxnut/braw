#include "ir/instruction.hpp"
#include "ir/instructions/binary.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/return.hpp"

void IRBuilder::build(const AST::ReturnNode* node, BrawContext& context, IRFunctionContext& ictx) {
    if(node->m_value) {
        Operator op = buildExpression(node->m_value.get(), context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, Register{"%return"}, op));
    }

    ictx.m_instructions.push_back(std::make_unique<Instruction>(Instruction::Return));
}