#include "et_builder.hpp"
#include "parser/nodes/variable_access.hpp"
#include "execution-tree/nodes/variable_access.hpp"

std::unique_ptr<EvaluatableNode> ETBuilder::buildVariableAccess(const AST::VariableAccessNode* node, BrawContext& context) {
    std::unique_ptr<VariableAccessNode> var = std::make_unique<VariableAccessNode>();

    ScopeInfo info = context.getScopeInfo(node->m_name).value();
    var->m_type = info.m_type;
    var->m_size = info.getSize();
    var->m_stackOffset = info.m_stackOffset;
    
    return var;
}