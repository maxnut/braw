#include "ir/instructions/binary.hpp"
#include "ir/label.hpp"
#include "ir/operator.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/if.hpp"
#include <cstdint>
#include <memory>

void IRBuilder::build(const AST::IfNode* node, BrawContext& context, IRFunctionContext& ictx) {
    Operator condition = buildExpression(node->m_condition.get(), context, ictx);
    Label label;
    label.m_id = "." + std::to_string((uintptr_t)node);
    ictx.m_instructions.push_back(
        std::make_unique<BinaryInstruction>(Instruction::JumpFalse, condition, label.m_id)
    );
    build(node->m_then.get(), context, ictx);
    ictx.m_instructions.push_back(std::make_unique<Label>(label));
}