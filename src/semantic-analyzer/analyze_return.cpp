#include "semantic_analyzer.hpp"
#include "parser/nodes/return.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::ReturnNode* node, BrawContext& ctx) {
    if(node->m_value) {
        auto errOpt = analyze(node->m_value.get(), ctx);
        if(errOpt) return errOpt;
    }

    if(node->m_value && getType(node->m_value.get(), ctx).value() != ctx.m_currentFunction->m_returnType)
        return mismatchedTypes(node, getType(node->m_value.get(), ctx).value().m_name, ctx.m_currentFunction->m_returnType.m_name);

    return std::nullopt;
}