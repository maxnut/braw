#include "semantic_analyzer.hpp"
#include "parser/nodes/return.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::ReturnNode* node, BrawContext& ctx) {
    if(node->m_value) {
        if(!expressionWhitelist.contains(node->m_value->m_type)) return invalidInstruction(node->m_value.get(), node, ctx);
        auto errOpt = analyze(node->m_value.get(), ctx);
        if(errOpt) return errOpt;
        ctx.m_returned = true;
    }

    if(node->m_value && getType(node->m_value.get(), ctx).value() != ctx.m_currentFunction->m_returnType)
        return mismatchedTypes(node, getType(node->m_value.get(), ctx).value().m_name, ctx.m_currentFunction->m_returnType.m_name, ctx);

    return std::nullopt;
}