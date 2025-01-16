#include "et_builder.hpp"
#include "execution-tree/nodes/return.hpp"
#include "parser/nodes/return.hpp"

std::unique_ptr<FunctionInstructionNode> ETBuilder::buildReturn(const AST::ReturnNode* node, BrawContext& context) {
    std::unique_ptr<ReturnNode> returnNode = std::make_unique<ReturnNode>();

    if(node->m_value)
        returnNode->m_value = buildEvaluatable(node->m_value.get(), context);

    return returnNode;
}