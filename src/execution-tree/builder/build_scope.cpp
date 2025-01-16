#include "et_builder.hpp"
#include "execution-tree/nodes/function_instruction.hpp"

#include <unordered_map>

std::unique_ptr<FunctionInstructionNode> ETBuilder::build(const AST::ScopeNode* node, BrawContext& context) {
    std::unique_ptr<ScopeNode> scope = std::make_unique<ScopeNode>();

    for(auto& instruction : node->m_instructions) {
        std::unique_ptr<Node> base = std::get<std::unique_ptr<Node>>(build(instruction.get(), context));
        std::unique_ptr<FunctionInstructionNode> instr = std::unique_ptr<FunctionInstructionNode>(static_cast<FunctionInstructionNode*>(base.release()));
        scope->m_instructions.push_back(std::move(instr));
    }

    return scope;
}