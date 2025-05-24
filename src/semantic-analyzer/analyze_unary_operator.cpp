#include "rules.hpp"
#include "utils.hpp"
#include "semantic_analyzer.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "type_info.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::UnaryOperatorNode* node, BrawContext& ctx) {
    if(!expressionWhitelist.contains(node->m_operand->m_type)) return invalidInstruction(node->m_operand.get(), node, ctx);
    if(node->m_expression && !expressionWhitelist.contains(node->m_expression->m_type)) return invalidInstruction(node->m_expression.get(), node, ctx);

    auto errorOpt = analyze(node->m_operand.get(), ctx);
    if(errorOpt) return errorOpt;

    if(node->m_operator != "!" && node->m_operator != "cast" && node->m_operator != "." && node->m_operator != "->" && node->m_operator != "&" && node->m_operator != "*" && node->m_operator != "[]" && node->m_operator != "pre++" && node->m_operator != "pre--" && node->m_operator != "post++" && node->m_operator != "post--")
        return unknownOperator(node, ctx);

    auto typeOr = getType(node->m_operand.get(), ctx);
    if(!typeOr) return typeOr.error();
    if((node->m_operator == "*" || node->m_operator == "->") && !Rules::isPtr(typeOr.value().m_name))
        return mismatchedTypes(node, typeOr.value().m_name, "pointer", ctx);

    if(node->m_operator == "!" && typeOr.value().m_name != BOOL_T)
        return mismatchedTypes(node, typeOr.value().m_name, BOOL_T, ctx);
        
    if(node->m_operator == "->" || node->m_operator == ".") {
        typeOr = getType(node->m_operand.get(), ctx);
        if(!typeOr) return typeOr.error();
        TypeInfo type = typeOr.value();
        if(node->m_operator == "->") {
            auto typeOpt = Utils::getRawType(type, ctx);
            if(!typeOpt) return unknownType(node->m_operand.get(), type.m_name, ctx);
            type = typeOpt.value();
        }
        else if(Rules::isPtr(type.m_name))
            return invalidOperator(node, ctx);

        if(!type.m_members.contains(Utils::getIdentifier(node->m_data)))
            return unknownMember(node, type.m_name, Utils::getIdentifier(node->m_data), ctx);
    }
    else if(node->m_operator == "cast") {
        std::string data = Utils::getIdentifier(node->m_data);
        while(Rules::isPtr(data)) {
            data = data.substr(0, data.size() - 1);
            if(!ctx.getTypeInfo(data).has_value())
                return unknownType(node, Utils::getIdentifier(node->m_data), ctx);
        }
        
        typeOr = getType(node->m_operand.get(), ctx);
        if(!typeOr) return typeOr.error();
        TypeInfo type = typeOr.value();

        if(!Rules::isPtr(Utils::getIdentifier(node->m_data))) {
            if(!Rules::isPtr(type.m_name) && !type.m_validCasts.contains(Utils::getIdentifier(node->m_data)))
                return invalidCast(node, type.m_name, ctx);
        }
        else if(type.m_name != INT_T && type.m_name != "long" && !Rules::isPtr(type.m_name))
            return invalidCast(node, type.m_name, ctx);
    }
    else if(node->m_operator == "[]") {
        errorOpt = analyze(node->m_expression.get(), ctx); 
        if(errorOpt) return errorOpt;
        typeOr = getType(node->m_operand.get(), ctx);
        if(!typeOr) return typeOr.error();
        if(!Rules::isPtr(typeOr.value().m_name))
            return mismatchedTypes(node, typeOr.value().m_name, "pointer", ctx);
        typeOr = getType(node->m_expression.get(), ctx);
        if(!typeOr) return typeOr.error();
        TypeInfo type = typeOr.value();
        if(type.m_name != INT_T && type.m_name != LONG_T)
            return mismatchedTypes(node, type.m_name, std::string(INT_T) + " or " + std::string(LONG_T), ctx);
    }
    else if(node->m_operator == "pre++" || node->m_operator == "pre--" || node->m_operator == "post++" || node->m_operator == "post--") {
        typeOr = getType(node->m_operand.get(), ctx);
        if(!typeOr) return typeOr.error();
        if(typeOr->m_name != INT_T && typeOr->m_name != LONG_T && typeOr->m_name != CHAR_T && typeOr->m_name != UINT_T && typeOr->m_name != ULONG_T && typeOr->m_name != UCHAR_T)
            return mismatchedTypes(node, typeOr.value().m_name, "integer", ctx);
    }

    return std::nullopt;
}