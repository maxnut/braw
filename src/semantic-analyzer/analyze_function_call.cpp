#include "semantic_analyzer.hpp"
#include "parser/nodes/function_call.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::FunctionCallNode* node, BrawContext& ctx) {
    std::vector<TypeInfo> parameters = std::vector<TypeInfo>();
    parameters.reserve(node->m_parameters.size());

    for(auto& param : node->m_parameters) {
        auto errorOpt = analyze(param.get(), ctx);
        if(errorOpt) return errorOpt;

        parameters.push_back(getType(param.get(), ctx).value());
    }

    if(!ctx.getFunction(node->m_name, parameters))
        return unknownFunction(node, parameters);

    return std::nullopt;
}