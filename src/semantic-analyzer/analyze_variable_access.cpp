#include "parser/nodes/identifier.hpp"
#include "semantic_analyzer.hpp"
#include "parser/nodes/variable_access.hpp"
#include "utils.hpp"
#include <memory>

std::optional<SemanticError> SemanticAnalyzer::analyze(AST::VariableAccessNode* node, BrawContext& ctx) {
    if(!ctx.isDefinedInScope(Utils::getIdentifier(node->m_name)))
        return unknownVariable(node, ctx);

    ScopeInfo info = ctx.getScopeInfo(Utils::getIdentifier(node->m_name)).value();
    if(info.m_retain) {
        auto id = std::static_pointer_cast<AST::IdentifierNode>(node->m_name);
        id->m_name += "_" + Utils::functionSignatureString(*ctx.m_currentFunction);
    }
    
    return std::nullopt;
}