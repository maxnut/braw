#include "semantic_analyzer.hpp"
#include "parser/nodes/return.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::ReturnNode* node, BrawContext& ctx) {
    if(node->m_value) {
        if(!expressionWhitelist.contains(node->m_value->m_type)) return invalidInstruction(node->m_value.get(), node, ctx);
        auto errOpt = analyze(node->m_value.get(), ctx);
        if(errOpt) return errOpt;
        ctx.m_returned = true;
    }

    auto typeOr = getType(node->m_value.get(), ctx);
    if(!typeOr) return typeOr.error();
    if(node->m_value && typeOr.value() != ctx.m_currentFunction->m_returnType)
        return mismatchedTypes(node, typeOr.value().m_name, ctx.m_currentFunction->m_returnType.m_name, ctx);

    return std::nullopt;
}