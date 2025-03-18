#include "rules.hpp"
#include "utils.hpp"
#include "semantic_analyzer.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "type_info.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::UnaryOperatorNode* node, BrawContext& ctx) {
    auto errorOpt = analyze(node->m_operand.get(), ctx);
    if(errorOpt) return errorOpt;

    if(node->m_operator != "cast" && node->m_operator != "." && node->m_operator != "->" && node->m_operator != "&" && node->m_operator != "*" && node->m_operator != "[]")
        return unknownOperator(node, ctx);

    if((node->m_operator == "*" || node->m_operator == "->") && !Rules::isPtr(getType(node->m_operand.get(), ctx).value().m_name))
        return mismatchedTypes(node, getType(node->m_operand.get(), ctx).value().m_name, "pointer", ctx);
        
    if(node->m_operator == "->" || node->m_operator == ".") {
        TypeInfo type = getType(node->m_operand.get(), ctx).value();
        if(node->m_operator == "->") {
            auto typeOpt = Utils::getRawType(type, ctx);
            if(!typeOpt) return unknownType(node->m_operand.get(), type.m_name, ctx);
            type = typeOpt.value();
        }

        if(!type.m_members.contains(node->m_data))
            return unknownMember(node, type.m_name, node->m_data, ctx);
    }
    else if(node->m_operator == "cast") {
        TypeInfo type = getType(node->m_operand.get(), ctx).value();

        if(!Rules::isPtr(node->m_data)) {
            if(!type.m_validCasts.contains(node->m_data))
                return invalidCast(node, type.m_name, ctx);
            }
        else if(type.m_name != INT_T && type.m_name != "long" && !Rules::isPtr(type.m_name))
            return invalidCast(node, type.m_name, ctx);
    }
    else if(node->m_operator == "[]") {
        errorOpt = analyze(node->m_expression.get(), ctx); 
        if(errorOpt) return errorOpt;
        if(!Rules::isPtr(getType(node->m_operand.get(), ctx).value().m_name))
            return mismatchedTypes(node, getType(node->m_operand.get(), ctx).value().m_name, "pointer", ctx);
        TypeInfo type = getType(node->m_expression.get(), ctx).value();
        if(type.m_name != INT_T && type.m_name != LONG_T)
            return mismatchedTypes(node, type.m_name, std::string(INT_T) + " or " + std::string(LONG_T), ctx);
    }

    return std::nullopt;
}