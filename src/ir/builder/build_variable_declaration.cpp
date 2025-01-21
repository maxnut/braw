#include "ir/instructions/binary.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/variable_declaration.hpp"

void IRBuilder::build(const AST::VariableDeclarationNode* node, BrawContext& context, Instructions& is) {
    Operator op = Value(0);

    if(node->m_value)
        op = buildExpression(node->m_value.get(), context, is);

    is.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, Register{node->m_name}, op));
}