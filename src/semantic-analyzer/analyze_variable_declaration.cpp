#include "semantic_analyzer.hpp"
#include "parser/nodes/variable_declaration.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::VariableDeclarationNode* node, BrawContext& ctx) {
    if(!ctx.getTypeInfo(node->m_type))
        return unknownType(node, node->m_type);

    ctx.m_scopes.back()[node->m_name] = ScopeInfo{
        ctx.getTypeInfo(node->m_type).value(),
        ctx.m_stackSize,
        0
    };
    ctx.m_stackSize += ctx.getTypeInfo(node->m_type)->m_size;
    
    return std::nullopt;
}