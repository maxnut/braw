#include "semantic_analyzer.hpp"
#include "parser/nodes/variable_access.hpp"
#include "utils.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(AST::VariableAccessNode* node, BrawContext& ctx) {
    if(!ctx.isDefinedInScope(node->m_name))
        return unknownVariable(node, ctx);

    ScopeInfo info = ctx.getScopeInfo(node->m_name).value();
    if(info.m_retain)
        node->m_name.m_name += "_" + Utils::functionSignatureString(*ctx.m_currentFunction);
    
    return std::nullopt;
}