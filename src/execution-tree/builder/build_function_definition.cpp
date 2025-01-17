#include "et_builder.hpp"
#include "execution-tree/nodes/function_definition.hpp"

std::shared_ptr<FunctionDefinitionNode> ETBuilder::buildFunctionDefinition(const AST::FunctionDefinitionNode* node, BrawContext& context) {
    std::vector<TypeInfo> parameters;
    for(auto& parameter : node->m_signature.m_parameters)
        parameters.push_back(context.getTypeInfo(parameter->m_type).value());

    std::shared_ptr<FunctionDefinitionNode> func = context.getFunction(node->m_signature.m_name, parameters);
    func->m_scope = std::move(buildScope(node->m_scope.get(), context));

    return func;
}