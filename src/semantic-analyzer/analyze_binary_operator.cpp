#include "semantic_analyzer.hpp"
#include "parser/nodes/binary_operator.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::BinaryOperatorNode* node, BrawContext& ctx) {
    auto errorOpt = analyze(node->m_left.get(), ctx);
    if(errorOpt) return errorOpt;

    errorOpt = analyze(node->m_right.get(), ctx);
    if(errorOpt) return errorOpt;

    TypeInfo leftType = getType(node->m_left.get(), ctx).value();
    TypeInfo rightType = getType(node->m_right.get(), ctx).value();

    if(leftType != rightType) return mismatchedTypes(node, leftType.m_name, rightType.m_name);
    if(!leftType.m_operators.contains(node->m_operator) && node->m_operator != "=") return unknownOperator(node);

    return std::nullopt;
}