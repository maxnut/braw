#include "rules.hpp"
#include "semantic_analyzer.hpp"
#include "parser/nodes/binary_operator.hpp"

bool ptrCheck(const TypeInfo& t1, const TypeInfo& t2) {
    return Rules::isPtr(t1.m_name) && (t2.m_name == INT_T || t2.m_name == LONG_T);
}

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::BinaryOperatorNode* node, BrawContext& ctx) {
    auto errorOpt = analyze(node->m_left.get(), ctx);
    if(errorOpt) return errorOpt;

    errorOpt = analyze(node->m_right.get(), ctx);
    if(errorOpt) return errorOpt;

    TypeInfo leftType = getType(node->m_left.get(), ctx).value();
    TypeInfo rightType = getType(node->m_right.get(), ctx).value();

    if(leftType != rightType && !ptrCheck(leftType, rightType) && !ptrCheck(rightType, leftType)) return mismatchedTypes(node, leftType.m_name, rightType.m_name);
    if(!hasOperator(leftType, node->m_operator) && node->m_operator != "=") return unknownOperator(node);

    return std::nullopt;
}