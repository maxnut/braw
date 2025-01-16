#include "semantic_analyzer.hpp"
#include "parser/nodes/unary_operator.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::UnaryOperatorNode* node, BrawContext& ctx) {
    auto errorOpt = analyze(node->m_operand.get(), ctx);
    if(errorOpt) return errorOpt;

    if(node->m_operator != "cast" && node->m_operator != "." && node->m_operator != "->" && node->m_operator != "&" && node->m_operator != "*")
        return unknownOperator(node);

    return std::nullopt;
}