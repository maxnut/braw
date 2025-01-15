#include "semantic_analyzer.hpp"
#include "parser/nodes/literal.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::LiteralNode* node, BrawContext& ctx) {
    return std::nullopt;
}