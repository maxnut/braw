#include "semantic_analyzer.hpp"
#include "parser/nodes/while.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::WhileNode* node, BrawContext& ctx) {
    if(!expressionWhitelist.contains(node->m_condition->m_type)) return invalidInstruction(node->m_condition.get(), node, ctx);

    auto errorOpt = analyze(node->m_condition.get(), ctx);
    if(errorOpt) return errorOpt;

    errorOpt = analyze(node->m_then.get(), ctx);
    if(errorOpt) return errorOpt;

    return std::nullopt;
}