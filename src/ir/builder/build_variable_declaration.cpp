#include "ir/instructions/binary.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/variable_declaration.hpp"

void IRBuilder::build(const AST::VariableDeclarationNode* node, BrawContext& context, IRFunctionContext& ictx) {
    Operator op = Value(0);

    if(node->m_value)
        op = buildExpression(node->m_value.get(), context, ictx);

    ictx.m_tables.back().insert({
        "%" + node->m_name.m_name,
        {node->m_type}
    });
    ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, Register{"%" + node->m_name.m_name}, op));
}