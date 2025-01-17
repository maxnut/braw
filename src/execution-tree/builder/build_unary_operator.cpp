#include "et_builder.hpp"
#include "parser/nodes/unary_operator.hpp"

std::unique_ptr<EvaluatableNode> ETBuilder::buildUnaryOperator(const AST::UnaryOperatorNode* node, BrawContext& context) {
    if(node->m_operator == "cast") return buildCast(node, context);
    else if(node->m_operator == "&") return buildAddress(node, context);
    else if(node->m_operator == "*") return buildDereference(node, context);
    else if(node->m_operator == "->") return buildArrow(node, context);
    else if(node->m_operator == ".") return buildDot(node, context);

    return nullptr;
}