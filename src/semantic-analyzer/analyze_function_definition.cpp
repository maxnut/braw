#include "braw_context.hpp"
#include "semantic_analyzer.hpp"
#include "parser/nodes/function_definition.hpp"
#include "type_info.hpp"
#include "utils.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::FunctionDefinitionNode* node, BrawContext& ctx) {
    if(!ctx.getTypeInfo(Utils::getIdentifier(node->m_signature.m_returnType)))
        return unknownType(node, Utils::getIdentifier(node->m_signature.m_returnType), ctx);

    std::shared_ptr<FunctionSignature> func = std::make_shared<FunctionSignature>();
    func->m_name = Utils::getIdentifier(node->m_signature.m_name);
    func->m_returnType = ctx.getTypeInfo(Utils::getIdentifier(node->m_signature.m_returnType)).value();
    func->m_external = node->m_signature.m_external;

    size_t initialStackSize = ctx.m_stackSize;

    std::unordered_map<std::string, ScopeInfo> scopeTable = std::unordered_map<std::string, ScopeInfo>();

    for(auto& param : node->m_signature.m_parameters) {
        if(!ctx.getTypeInfo(Utils::getIdentifier(param->m_type)))
            return unknownType(node, Utils::getIdentifier(param->m_type) , ctx);
        func->m_parameters.push_back(ctx.getTypeInfo(Utils::getIdentifier(param->m_type)).value());
        func->m_parameterNames.push_back(Utils::getIdentifier(param->m_name));

        scopeTable[Utils::getIdentifier(param->m_name)] = ScopeInfo{
            ctx.getTypeInfo(Utils::getIdentifier(param->m_type)).value(),
            ctx.m_stackSize,
            0
        };
        ctx.m_stackSize += ctx.getTypeInfo(Utils::getIdentifier(param->m_type))->m_size;
    }

    if(ctx.functionExists(func))
        return duplicateFunction(node, node->m_signature, ctx);

    ctx.m_functionTable[func->m_name].push_back(func);
    ctx.m_currentFunction = func;

    if(!func->m_external && node->m_scope) {
        ctx.m_scopes.push_back(scopeTable);
        std::optional<SemanticError> errOpt = analyze(node->m_scope.get(), ctx);
        ctx.m_scopes.pop_back();
        if(errOpt) return errOpt;
        if(func->m_returnType.m_name != VOID_T && !ctx.m_returned)
            return missingReturn(node, node->m_signature, ctx);
        ctx.m_returned = false;
    }

    ctx.m_stackSize = initialStackSize;

    return std::nullopt;
}