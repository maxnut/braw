#include "et_builder.hpp"
#include "execution-tree/nodes/scope.hpp"

std::unique_ptr<ScopeNode> ETBuilder::buildScope(const AST::ScopeNode* node, BrawContext& context) {
    std::unique_ptr<ScopeNode> scope = std::make_unique<ScopeNode>();
    size_t initialStackSize = context.m_stackSize;
    context.m_scopes.push_back(std::unordered_map<std::string, ScopeInfo>());

    for(auto& instruction : node->m_instructions)
        scope->m_instructions.push_back(std::move(buildInstruction(instruction.get(), context)));

    context.m_scopes.pop_back();
    context.m_stackSize = initialStackSize;

    return scope;
}