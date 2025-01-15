#include "semantic_analyzer.hpp"
#include "parser/nodes/variable_access.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::VariableAccessNode* node, BrawContext& ctx) {
    if(!ctx.isDefined(node->m_name))
        return unknownVariable(node);
    
    return std::nullopt;
}