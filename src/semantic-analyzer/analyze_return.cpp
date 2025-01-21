#include "semantic_analyzer.hpp"
#include "parser/nodes/return.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::ReturnNode* node, BrawContext& ctx) {
    if(node->m_value) {
        auto errOpt = analyze(node->m_value.get(), ctx);
        if(errOpt) return errOpt;
    }

    return std::nullopt;
}