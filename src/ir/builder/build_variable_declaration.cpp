#include "ir/instructions/binary.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/variable_declaration.hpp"

void IRBuilder::build(const AST::VariableDeclarationNode* node, BrawContext& context, IRFunctionContext& ictx) {
    Operator op = Value(0);

    //TODO: check type size, put in stack if too big!

    if(node->m_value)
        op = buildExpression(node->m_value.get(), context, ictx);

    ictx.m_tables.back().insert({
        node->m_name,
        {node->m_type, "%" + std::to_string((uintptr_t)node)}
    });
    ictx.m_tables.back().insert({
        "%" + std::to_string((uintptr_t)node),
        {node->m_type, "%" + std::to_string((uintptr_t)node)}
    });
    Register r{"%" + std::to_string((uintptr_t)node)};
    ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, r, op));
}