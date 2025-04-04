#include "ir/instructions/basic.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/variable_declaration.hpp"

void IRBuilder::build(const AST::VariableDeclarationNode* node, BrawContext& context, IRFunctionContext& ictx) {
    auto reg = makeOrGetRegister("%" + node->m_name.m_name + "_" + std::to_string(ictx.m_scopeDepth), ictx);
    reg->m_type = context.getTypeInfo(node->m_type).value();
    reg->m_registerType = getRegisterType(reg->m_type);
    reg->m_scale = node->m_scale;
    reg->m_type.m_builtin = reg->m_scale <= 1 && reg->m_type.m_builtin;

    if(!reg->m_type.m_builtin)
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Allocate, node->m_rangeBegin, reg, Value((long)reg->m_type.m_size)));

    if(node->m_value) {
        Operand op = buildExpression(node->m_value.get(), context, ictx);
        if(std::holds_alternative<Value>(op) && std::holds_alternative<std::string>(std::get<Value>(op))) {
            ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Point, node->m_value->m_rangeBegin, reg, op));
            return;
        }
        moveToRegister("%" + node->m_name.m_name + "_" + std::to_string(ictx.m_scopeDepth), op, node->m_value->m_rangeBegin, context, ictx);
    }
}