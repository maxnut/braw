#include "et_builder.hpp"
#include "execution-tree/nodes/dereference.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "utils.hpp"

std::unique_ptr<EvaluatableNode> ETBuilder::buildDereference(const AST::UnaryOperatorNode* node, BrawContext& context) {
    std::unique_ptr<DereferenceNode> dereference = std::make_unique<DereferenceNode>();

    dereference->m_base = buildEvaluatable(node->m_operand.get(), context);
    dereference->m_type = Utils::getRawType(dereference->m_base->m_type, context).value();
    dereference->m_size = dereference->m_type.m_size;

    return dereference;
}