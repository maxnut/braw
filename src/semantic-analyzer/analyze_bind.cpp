#include "semantic_analyzer.hpp"
#include "parser/nodes/bind.hpp"
#include "execution-tree/nodes/native_function.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::BindNode* node, BrawContext& ctx) {
    for(auto& func : node->m_functions) {
        const AST::FunctionSignature& sig = func.second;
        if(!ctx.getTypeInfo(sig.m_returnType))
            return unknownType(node, sig.m_returnType);

        std::shared_ptr<NativeFunctionNode> nativeFunc = std::make_shared<NativeFunctionNode>();
        nativeFunc->m_signature.m_name = sig.m_name;
        nativeFunc->m_signature.m_returnType = ctx.getTypeInfo(sig.m_returnType).value();

        for(auto& param : sig.m_parameters) {
            if(!ctx.getTypeInfo(param->m_type.m_name))
                return unknownType(node, param->m_type.m_name);
            nativeFunc->m_signature.m_parameters.push_back(ctx.getTypeInfo(param->m_type).value());
            nativeFunc->m_signature.m_parameterNames.push_back(param->m_name);
        }

        if(ctx.functionExists(nativeFunc))
            return duplicateFunction(node, sig);

        ctx.m_functionTable[nativeFunc->m_signature.m_name].push_back(nativeFunc);
    }

    return std::nullopt;
}