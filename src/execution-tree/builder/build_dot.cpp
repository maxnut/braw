#include "et_builder.hpp"
#include "execution-tree/nodes/dot.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "type_info.hpp"

std::unique_ptr<EvaluatableNode> ETBuilder::buildDot(const AST::UnaryOperatorNode* node, BrawContext& context) {
    std::unique_ptr<DotNode> dot = std::make_unique<DotNode>();

    dot->m_base = buildEvaluatable(node->m_operand.get(), context);
    MemberInfo info = dot->m_base->m_type.m_members[node->m_data];
    dot->m_type = context.getTypeInfo(info.m_type).value();
    dot->m_size = dot->m_type.m_size;
    dot->m_offset = info.m_offset;

    return dot;
}