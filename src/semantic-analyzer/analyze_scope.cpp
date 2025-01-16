#include "semantic_analyzer.hpp"
#include "parser/nodes/scope.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::ScopeNode* node, BrawContext& ctx) {
    size_t initialStackSize = ctx.m_stackSize;
    ctx.m_scopes.push_front(std::unordered_map<std::string, ScopeInfo>());

    for(auto& instruction : node->m_instructions) {
        std::optional<SemanticError> errOpt = analyze(instruction.get(), ctx);
        if(errOpt) return errOpt;
    }
    ctx.m_scopes.pop_front();
    ctx.m_stackSize = initialStackSize;

    return std::nullopt;
}