#include "et_builder.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "execution-tree/nodes/cast.hpp"

std::unique_ptr<EvaluatableNode> ETBuilder::buildCast(const AST::UnaryOperatorNode* node, BrawContext& context) {
    std::unique_ptr<CastNode> cast = std::make_unique<CastNode>();

    cast->m_base = buildEvaluatable(node, context);
    cast->m_type = context.getTypeInfo(node->m_data).value();
    cast->m_function = cast->m_type.m_casts[cast->m_type.m_name].m_function;
    cast->m_size = cast->m_type.m_size;
    
    return cast;
}