#include "et_builder.hpp"
#include "execution-tree/nodes/function_definition.hpp"

std::shared_ptr<FunctionDefinitionNode> ETBuilder::buildFunctionDefinition(const AST::FunctionDefinitionNode* node, BrawContext& context) {
    std::vector<TypeInfo> parameters;
    std::unordered_map<std::string, ScopeInfo> scopeTable = std::unordered_map<std::string, ScopeInfo>();
    size_t initialStackSize = context.m_stackSize;

    for(auto& parameter : node->m_signature.m_parameters) {
        TypeInfo t = context.getTypeInfo(parameter->m_type).value();
        parameters.push_back(t);
        scopeTable[parameter->m_name] = ScopeInfo{t, context.m_stackSize, 0};
        context.m_stackSize += t.m_size;
    }

    std::shared_ptr<FunctionDefinitionNode> func = context.getFunction(node->m_signature.m_name, parameters);
    context.m_scopes.push_back(scopeTable);
    func->m_scope = std::move(buildScope(node->m_scope.get(), context));
    context.m_scopes.pop_back();
    context.m_stackSize = initialStackSize;

    return func;
}