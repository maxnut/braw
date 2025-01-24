#include "ir_builder.hpp"
#include "parser/nodes/variable_declaration.hpp"

void IRBuilder::build(const AST::VariableDeclarationNode* node, BrawContext& context, IRFunctionContext& ictx) {
    Operator op = Value(0);

    //TODO: check type size, put in stack if too big!

    if(node->m_value)
        op = buildExpression(node->m_value.get(), context, ictx);

    moveToRegister("%" + node->m_name.m_name + "_" + std::to_string(ictx.m_scopeDepth), op, context, ictx);
}