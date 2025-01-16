#include "semantic_analyzer.hpp"
#include "parser/nodes/function_definition.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::FunctionDefinitionNode* node, BrawContext& ctx) {
    if(!ctx.m_typeTable.contains(node->m_signature.m_returnType))
        return unknownType(node, node->m_signature.m_returnType);

    std::shared_ptr<FunctionDefinitionNode> func = std::make_shared<FunctionDefinitionNode>();
    func->m_signature.m_name = node->m_signature.m_name;
    func->m_signature.m_returnType = ctx.m_typeTable[node->m_signature.m_returnType];

    size_t initialStackSize = ctx.m_stackSize;

    std::unordered_map<std::string, ScopeInfo> scopeTable = std::unordered_map<std::string, ScopeInfo>();

    for(auto& param : node->m_signature.m_parameters) {
        if(!ctx.m_typeTable.contains(param.m_type.m_name))
            return unknownType(node, param.m_type.m_name);
        func->m_signature.m_parameters.push_back(ctx.m_typeTable[param.m_type]);
        func->m_signature.m_parameterNames.push_back(param.m_name);

        scopeTable[param.m_name] = ScopeInfo{
            ctx.m_typeTable[param.m_type],
            ctx.m_stackSize,
            0
        };
        ctx.m_stackSize += ctx.m_typeTable[param.m_type].m_size;
    }

    if(ctx.functionExists(func))
        return duplicateFunction(node);

    ctx.m_scopes.push_front(scopeTable);
    std::optional<SemanticError> errOpt = analyze(node->m_scope.get(), ctx);
    ctx.m_scopes.pop_front();
    if(errOpt) return errOpt;

    ctx.m_stackSize = initialStackSize;

    return std::nullopt;
}