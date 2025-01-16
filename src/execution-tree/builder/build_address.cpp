#include "et_builder.hpp"
#include "execution-tree/nodes/address.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "utils.hpp"

std::unique_ptr<EvaluatableNode> ETBuilder::buildAddress(const AST::UnaryOperatorNode* node, BrawContext& context) {
    std::unique_ptr<AddressNode> address = std::make_unique<AddressNode>();

    address->m_base = buildEvaluatable(node->m_operand.get(), context);
    address->m_type = Utils::makePointer(address->m_base->m_type);
    address->m_size = address->m_type.m_size;

    return address;
}