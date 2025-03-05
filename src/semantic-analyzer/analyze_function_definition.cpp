#include "braw_context.hpp"
#include "semantic_analyzer.hpp"
#include "parser/nodes/function_definition.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::FunctionDefinitionNode* node, BrawContext& ctx) {
    if(!ctx.getTypeInfo(node->m_signature.m_returnType))
        return unknownType(node, node->m_signature.m_returnType);

    std::shared_ptr<FunctionSignature> func = std::make_shared<FunctionSignature>();
    func->m_name = node->m_signature.m_name;
    func->m_returnType = ctx.getTypeInfo(node->m_signature.m_returnType).value();

    size_t initialStackSize = ctx.m_stackSize;

    std::unordered_map<std::string, ScopeInfo> scopeTable = std::unordered_map<std::string, ScopeInfo>();

    for(auto& param : node->m_signature.m_parameters) {
        if(!ctx.getTypeInfo(param->m_type.m_name))
            return unknownType(node, param->m_type.m_name);
        func->m_parameters.push_back(ctx.getTypeInfo(param->m_type).value());
        func->m_parameterNames.push_back(param->m_name);

        scopeTable[param->m_name] = ScopeInfo{
            ctx.getTypeInfo(param->m_type).value(),
            ctx.m_stackSize,
            0
        };
        ctx.m_stackSize += ctx.getTypeInfo(param->m_type)->m_size;
    }

    if(ctx.functionExists(func))
        return duplicateFunction(node, node->m_signature);

    ctx.m_functionTable[func->m_name].push_back(func);
    ctx.m_currentFunction = func;

    ctx.m_scopes.push_back(scopeTable);
    std::optional<SemanticError> errOpt = analyze(node->m_scope.get(), ctx);
    ctx.m_scopes.pop_back();
    if(errOpt) return errOpt;

    ctx.m_stackSize = initialStackSize;

    return std::nullopt;
}