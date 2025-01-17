#include "et_builder.hpp"
#include "execution-tree/nodes/native_function.hpp"
#include "parser/nodes/function_definition.hpp"
#include "binder/binder.hpp"
#include "type_info.hpp"
#include <memory>

std::vector<std::shared_ptr<FunctionDefinitionNode>> ETBuilder::buildBind(const AST::BindNode* ast, BrawContext& context) {
    std::vector<std::shared_ptr<FunctionDefinitionNode>> functions;

    for(auto& func : ast->m_functions) {
        const AST::FunctionSignature& sig = func.second;

        std::vector<TypeInfo> parameters;

        for(auto& p : sig.m_parameters)
            parameters.push_back(context.getTypeInfo(p->m_type).value());

        std::shared_ptr<NativeFunctionNode> function = std::static_pointer_cast<NativeFunctionNode>(context.getFunction(sig.m_name, parameters));
        function->m_function = Binder::getFunction({"./"}, ast->m_library, func.first);
        functions.push_back(function);
    }

    return functions;
}