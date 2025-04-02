#include "rules.hpp"
#include "semantic_analyzer.hpp"
#include "parser/nodes/variable_declaration.hpp"
#include "utils.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(AST::VariableDeclarationNode* node, BrawContext& ctx) {
    if(node->m_type.m_name == "@infer") {
        if(!node->m_value)
            return cannotInferType(node, ctx);
        auto inferOr = getType(node->m_value.get(), ctx);
        if(!inferOr) return inferOr.error();
        node->m_type = inferOr.value().m_name;
    }

    auto typeOpt = ctx.getTypeInfo(node->m_type);
    if(!typeOpt)
        return unknownType(node, node->m_type, ctx);

    if(node->m_scale > 1 && !Rules::isPtr(typeOpt.value().m_name))
        return mismatchedTypes(node, typeOpt.value().m_name, Utils::makePointer(typeOpt.value()).m_name, ctx);

    if(node->m_value) {
        auto errorOpt = analyze(node->m_value.get(), ctx);
        if(errorOpt) return errorOpt;
        auto typeOr = getType(node->m_value.get(), ctx);
        if(!typeOr) return typeOr.error();
        TypeInfo type = typeOr.value();

        if(node->m_scale > 1)
            return mismatchedTypes(node, type.m_name, "array", ctx);
        if(type != typeOpt.value())
            return mismatchedTypes(node, type.m_name, typeOpt.value().m_name, ctx);
    }

    ctx.m_scopes.back()[node->m_name] = ScopeInfo{
        ctx.getTypeInfo(node->m_type).value(),
        ctx.m_stackSize,
        0
    };
    ctx.m_stackSize += node->m_scale <= 1 ? typeOpt->m_size : Utils::getRawType(typeOpt.value(), ctx)->m_size * typeOpt->m_size;
    
    return std::nullopt;
}