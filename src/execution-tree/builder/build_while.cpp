#include "et_builder.hpp"
#include "execution-tree/nodes/while.hpp"
#include "parser/nodes/while.hpp"

std::unique_ptr<FunctionInstructionNode> ETBuilder::buildWhile(const AST::WhileNode* node, BrawContext& context) {
    std::unique_ptr<WhileNode> whileNode = std::make_unique<WhileNode>();

    whileNode->m_condition = buildEvaluatable(node->m_condition.get(), context);
    whileNode->m_scope = buildScope(node->m_then.get(), context);

    return whileNode;
}