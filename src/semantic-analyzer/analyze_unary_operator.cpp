#include "rules.hpp"
#include "utils.hpp"
#include "semantic_analyzer.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "type_info.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::UnaryOperatorNode* node, BrawContext& ctx) {
    auto errorOpt = analyze(node->m_operand.get(), ctx);
    if(errorOpt) return errorOpt;

    if(node->m_operator != "cast" && node->m_operator != "." && node->m_operator != "->" && node->m_operator != "&" && node->m_operator != "*")
        return unknownOperator(node);

    if((node->m_operator == "*" || node->m_operator == "->") && !Rules::isPtr(getType(node->m_operand.get(), ctx).value().m_name))
        return mismatchedTypes(node, getType(node->m_operand.get(), ctx).value().m_name, "pointer");
        
    if(node->m_operator == "->" || node->m_operator == ".") {
        TypeInfo type = getType(node->m_operand.get(), ctx).value();
        if(node->m_operator == "->") {
            auto typeOpt = Utils::getRawType(type, ctx);
            if(!typeOpt) return unknownType(node->m_operand.get(), type.m_name);
            type = typeOpt.value();
        }

        if(!type.m_members.contains(node->m_data))
            return unknownMember(node, type.m_name, node->m_data);
    }

    return std::nullopt;
}