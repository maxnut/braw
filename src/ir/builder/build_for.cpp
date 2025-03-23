#include "ir/instructions/basic.hpp"
#include "ir/label.hpp"
#include "ir/operand.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/for.hpp"
#include <cstdint>
#include <memory>

void IRBuilder::build(const AST::ForNode* node, BrawContext& context, IRFunctionContext& ictx) {
    build(node->m_initializer.get(), context, ictx);
    Label labelCondition;
    labelCondition.m_id = "." + std::to_string((uintptr_t)node) + "_condition";
    Label labelBody;
    labelBody.m_id = "." + std::to_string((uintptr_t)node) + "_body";
    ictx.m_instructions.push_back(
        std::make_unique<BasicInstruction>(Instruction::Jump, labelCondition)
    );
    ictx.m_instructions.push_back(std::make_unique<Label>(labelBody));
    build(node->m_body.get(), context, ictx);
    build(node->m_increment.get(), context, ictx);
    ictx.m_instructions.push_back(std::make_unique<Label>(labelCondition));
    Operand condition = buildExpression(node->m_condition.get(), context, ictx);
    ictx.m_instructions.push_back(
        std::make_unique<BasicInstruction>(Instruction::JumpTrue, condition, labelBody)
    );
}