#include "semantic_analyzer.hpp"
#include "parser/nodes/variable_access.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::VariableAccessNode* node, BrawContext& ctx) {
    if(!ctx.isDefinedInScope(node->m_name))
        return unknownVariable(node);
    
    return std::nullopt;
}