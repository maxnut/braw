#include "semantic_analyzer.hpp"
#include "parser/nodes/variable_declaration.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::VariableDeclarationNode* node, BrawContext& ctx) {
    auto typeOpt = ctx.getTypeInfo(node->m_type);
    if(!typeOpt)
        return unknownType(node, node->m_type);

    if(node->m_value) {
        auto errorOpt = analyze(node->m_value.get(), ctx);
        if(errorOpt) return errorOpt;
        TypeInfo type = getType(node->m_value.get(), ctx).value();

        if(type != typeOpt.value())
            return mismatchedTypes(node, type.m_name, typeOpt.value().m_name);
    }

    ctx.m_scopes.back()[node->m_name] = ScopeInfo{
        ctx.getTypeInfo(node->m_type).value(),
        ctx.m_stackSize,
        0
    };
    ctx.m_stackSize += ctx.getTypeInfo(node->m_type)->m_size;
    
    return std::nullopt;
}