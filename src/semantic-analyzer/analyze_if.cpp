#include "semantic_analyzer.hpp"
#include "parser/nodes/if.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::IfNode* node, BrawContext& ctx) {
    auto errorOpt = analyze(node->m_condition.get(), ctx);
    if(errorOpt) return errorOpt;

    errorOpt = analyze(node->m_then.get(), ctx);
    if(errorOpt) return errorOpt;

    if(node->m_else) {
        errorOpt = analyze(node->m_else.get(), ctx);
        if(errorOpt) return errorOpt;
    }

    return std::nullopt;
}