#include "et_builder.hpp"
#include "execution-tree/nodes/if.hpp"
#include "parser/nodes/if.hpp"

std::unique_ptr<FunctionInstructionNode> ETBuilder::buildIf(const AST::IfNode* node, BrawContext& context) {
    std::unique_ptr<IfNode> ifNode = std::make_unique<IfNode>();

    ifNode->m_condition = buildEvaluatable(node->m_condition.get(), context);
    ifNode->m_scope = buildScope(node->m_then.get(), context);
    if(node->m_else)
        ifNode->m_else = buildEvaluatable(node->m_else.get(), context);

    return ifNode;
}