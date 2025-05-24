#include "parser/nodes/identifier.hpp"
#include "rules.hpp"
#include "semantic_analyzer.hpp"
#include "parser/nodes/variable_declaration.hpp"
#include "utils.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(AST::VariableDeclarationNode* node, BrawContext& ctx) {
    if(Utils::getIdentifier(node->m_type) == "@infer") {
        if(!node->m_value)
            return cannotInferType(node, ctx);
        auto inferOr = getType(node->m_value.get(), ctx);
        if(!inferOr) return inferOr.error();
        ((AST::IdentifierNode*)node->m_type.get())->m_name = inferOr.value().m_name;
    }

    auto typeOpt = ctx.getTypeInfo(Utils::getIdentifier(node->m_type));
    if(!typeOpt)
        return unknownType(node, Utils::getIdentifier(node->m_type), ctx);

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

    ctx.m_scopes.back()[Utils::getIdentifier(node->m_name)] = ScopeInfo{
        ctx.getTypeInfo(Utils::getIdentifier(node->m_type)).value(),
        ctx.m_stackSize,
        0,
        node->m_retain
    };
    ctx.m_stackSize += node->m_scale <= 1 ? typeOpt->m_size : Utils::getRawType(typeOpt.value(), ctx)->m_size * typeOpt->m_size;

    if(node->m_retain) {
        ((AST::IdentifierNode*)node->m_name.get())->m_name += "_" + Utils::functionSignatureString(*ctx.m_currentFunction);
        ctx.m_scopes.back()[Utils::getIdentifier(node->m_name)] = ScopeInfo{
        ctx.getTypeInfo(Utils::getIdentifier(node->m_type)).value(),
        ctx.m_stackSize,
        0,
        node->m_retain
    };
    }
    
    return std::nullopt;
}