#include "ir/instructions/binary.hpp"
#include "ir/label.hpp"
#include "ir/operator.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/if.hpp"
#include <cstdint>
#include <memory>

void IRBuilder::build(const AST::IfNode* node, BrawContext& context, Instructions& is) {
    Operator condition = buildExpression(node->m_condition.get(), context, is);
    Label label;
    label.m_id = "." + std::to_string((uintptr_t)node);
    is.push_back(
        std::make_unique<BinaryInstruction>(Instruction::JumpFalse, condition, label.m_id)
    );
    build(node->m_then.get(), context, is);
    is.push_back(std::make_unique<Label>(label));
}