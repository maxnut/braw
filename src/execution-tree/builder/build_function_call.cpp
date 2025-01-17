#include "et_builder.hpp"
#include "execution-tree/nodes/function_call.hpp"
#include "execution-tree/nodes/native_function_call.hpp"
#include "parser/nodes/function_call.hpp"
#include <memory>

std::unique_ptr<EvaluatableNode> ETBuilder::buildFunctionCall(const AST::FunctionCallNode* node, BrawContext& context) {
    std::vector<std::unique_ptr<EvaluatableNode>> parameters;

    for(auto& p : node->m_parameters)
        parameters.push_back(std::move(buildEvaluatable(p.get(), context)));

    auto function = context.getFunction(node->m_name, parameters);

    if(function->isNative()) {
        std::unique_ptr<NativeFunctionCallNode> nativeCall = std::make_unique<NativeFunctionCallNode>();
        nativeCall->m_function = function;
        nativeCall->m_parameters = std::move(parameters);
        nativeCall->m_type = function->m_signature.m_returnType;
        nativeCall->m_size = function->m_signature.m_returnType.m_size;
        return nativeCall;
    }

    std::unique_ptr<FunctionCallNode> call = std::make_unique<FunctionCallNode>();
    call->m_function = function;
    call->m_parameters = std::move(parameters);
    call->m_type = function->m_signature.m_returnType;
    call->m_size = function->m_signature.m_returnType.m_size;

    return call;
}