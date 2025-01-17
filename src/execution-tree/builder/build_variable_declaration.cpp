#include "et_builder.hpp"
#include "execution-tree/nodes/variable_declaration.hpp"
#include "parser/nodes/variable_declaration.hpp"

std::unique_ptr<FunctionInstructionNode> ETBuilder::buildVariableDeclaration(const AST::VariableDeclarationNode* node, BrawContext& context) {
    std::unique_ptr<VariableDeclarationNode> variableDeclaration = std::make_unique<VariableDeclarationNode>();

    variableDeclaration->m_type = context.getTypeInfo(node->m_type).value();
    if(node->m_value)
        variableDeclaration->m_assignmentValue = buildEvaluatable(node->m_value.get(), context);

    context.m_scopes.back()[node->m_name] = ScopeInfo{
        context.getTypeInfo(node->m_type).value(),
        context.m_stackSize,
        0
    };
    context.m_stackSize += context.getTypeInfo(node->m_type)->m_size;

    return variableDeclaration;
}