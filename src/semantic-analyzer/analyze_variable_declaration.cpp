#include "semantic_analyzer.hpp"
#include "parser/nodes/variable_declaration.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::VariableDeclarationNode* node, BrawContext& ctx) {
    if(!ctx.m_typeTable.contains(node->m_type))
        return unknownType(node, node->m_type);

    ctx.m_scopes.front()[node->m_type] = ScopeInfo{
        ctx.m_typeTable[node->m_type],
        ctx.m_stackSize,
        0
    };
    ctx.m_stackSize += ctx.m_typeTable[node->m_type].m_size;
    
    return std::nullopt;
}