#include "semantic_analyzer.hpp"
#include "parser/nodes/for.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::ForNode* node, BrawContext& ctx) {
    auto errorOpt = analyze(node->m_initializer.get(), ctx);
    if(errorOpt) return errorOpt;

    errorOpt = analyze(node->m_condition.get(), ctx);
    if(errorOpt) return errorOpt;

    errorOpt = analyze(node->m_increment.get(), ctx);
    if(errorOpt) return errorOpt;

    errorOpt = analyze(node->m_body.get(), ctx);
    if(errorOpt) return errorOpt;

    return std::nullopt;
}