#include "et_builder.hpp"
#include "execution-tree/nodes/arrow.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "type_info.hpp"
#include "utils.hpp"

std::unique_ptr<EvaluatableNode> ETBuilder::buildArrow(const AST::UnaryOperatorNode* node, BrawContext& context) {
    std::unique_ptr<ArrowNode> arrow = std::make_unique<ArrowNode>();

    arrow->m_base = buildEvaluatable(node->m_operand.get(), context);
    MemberInfo info = arrow->m_base->m_type.m_members[node->m_data];
    arrow->m_type = Utils::getRawType(context.getTypeInfo(info.m_type).value(), context).value();
    arrow->m_size = arrow->m_type.m_size;
    arrow->m_offset = info.m_offset;

    return arrow;
}