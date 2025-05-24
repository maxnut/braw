#include "semantic_analyzer.hpp"
#include "parser/nodes/function_call.hpp"
#include "utils.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::FunctionCallNode* node, BrawContext& ctx) {
    std::vector<TypeInfo> parameters = std::vector<TypeInfo>();
    parameters.reserve(node->m_parameters.size());

    for(auto& param : node->m_parameters) {
        if(!expressionWhitelist.contains(param->m_type)) return invalidInstruction(param.get(), node, ctx);
        auto errorOpt = analyze(param.get(), ctx);
        if(errorOpt) return errorOpt;

        auto typeOr = getType(param.get(), ctx);
        if(!typeOr) return typeOr.error();
        parameters.push_back(typeOr.value());
    }

    if(!ctx.getFunction(Utils::getIdentifier(node->m_name), parameters))
        return unknownFunction(node, parameters, ctx);

    return std::nullopt;
}