#include "ir_builder.hpp"
#include "parser/nodes/scope.hpp"

void IRBuilder::build(const AST::ScopeNode* node, BrawContext& context, Instructions& is) {
    for(auto& in : node->m_instructions)
        build(in.get(), context, is);
}