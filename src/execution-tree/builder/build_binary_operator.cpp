#include "et_builder.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "execution-tree/nodes/binary_operator.hpp"

std::unique_ptr<EvaluatableNode> ETBuilder::buildBinaryOperator(const AST::BinaryOperatorNode* node, BrawContext& context) {
    std::unique_ptr<BinaryOperatorNode> op = std::make_unique<BinaryOperatorNode>();
    op->m_left = buildEvaluatable(node->m_left.get(), context);
    op->m_right = buildEvaluatable(node->m_right.get(), context);
    op->m_type = context.getTypeInfo(op->m_left->m_type.m_operators[node->m_operator].m_returnType).value();
    op->m_function = op->m_left->m_type.m_operators[node->m_operator].m_function;
    op->m_size = op->m_type.m_size;
    
    return op;
}