#include "ir/instructions/basic.hpp"
#include "ir/label.hpp"
#include "ir/operand.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/while.hpp"
#include <cstdint>
#include <memory>

void IRBuilder::build(const AST::WhileNode* node, BrawContext& context, IRFunctionContext& ictx) {
    Label labelCondition{node->m_condition->m_rangeBegin};
    labelCondition.m_id = "." + std::to_string((uintptr_t)node) + "_condition";
    Label labelBody{node->m_then->m_rangeBegin};
    labelBody.m_id = "." + std::to_string((uintptr_t)node) + "_body";
    if(!node->m_do)
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Jump, node->m_rangeBegin, labelCondition));
    ictx.m_instructions.push_back(std::make_unique<Label>(labelBody));
    build(node->m_then.get(), context, ictx);
    ictx.m_instructions.push_back(std::make_unique<Label>(labelCondition));
    Operand condition = buildExpression(node->m_condition.get(), context, ictx);
    ictx.m_instructions.push_back(
        std::make_unique<BasicInstruction>(Instruction::JumpTrue, node->m_rangeEnd, condition, labelBody)
    );
}