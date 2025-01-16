#include "et_builder.hpp"
#include "execution-tree/nodes/scope.hpp"

std::unique_ptr<ScopeNode> ETBuilder::buildScope(const AST::ScopeNode* node, BrawContext& context) {
    std::unique_ptr<ScopeNode> scope = std::make_unique<ScopeNode>();

    for(auto& instruction : node->m_instructions)
        scope->m_instructions.push_back(std::move(buildInstruction(instruction.get(), context)));

    return scope;
}