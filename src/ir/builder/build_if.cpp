#include "ir/instructions/basic.hpp"
#include "ir/label.hpp"
#include "ir/operand.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/if.hpp"
#include <cstdint>
#include <memory>

void IRBuilder::build(const AST::IfNode* node, BrawContext& context, IRFunctionContext& ictx) {
    Operand condition = buildExpression(node->m_condition.get(), context, ictx);
    Label label{node->m_then->m_rangeEnd};
    label.m_id = "." + std::to_string((uintptr_t)node);
    ictx.m_instructions.push_back(
        std::make_unique<BasicInstruction>(Instruction::JumpFalse, node->m_condition->m_rangeBegin, condition, label)
    );
    build(node->m_then.get(), context, ictx);
    if(node->m_else) {
        Label elseLabel{node->m_else->m_rangeEnd};
        elseLabel.m_id = "." + std::to_string((uintptr_t)node) + "_else";
        ictx.m_instructions.push_back(
            std::make_unique<BasicInstruction>(Instruction::Jump, node->m_else->m_rangeBegin, elseLabel)
        );
        ictx.m_instructions.push_back(std::make_unique<Label>(label));
        build(node->m_else.get(), context, ictx);
        ictx.m_instructions.push_back(std::make_unique<Label>(elseLabel));
        return;
    }
    ictx.m_instructions.push_back(std::make_unique<Label>(label));
}