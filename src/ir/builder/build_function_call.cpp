#include "ir_builder.hpp"
#include "ir/instructions/call.hpp"
#include "parser/nodes/function_call.hpp"

Operator IRBuilder::buildCall(const AST::FunctionCallNode* node, BrawContext& context, IRFunctionContext& ictx) {
    CallInstruction call;
    call.m_id = node->m_name;

    std::vector<TypeInfo> tmpTypes;

    for(auto& param : node->m_parameters) {
        Operator op = buildExpression(param.get(), context, ictx);
        call.m_parameters.push_back(op);
        tmpTypes.push_back(getOperatorType(op, context, ictx));
    }

    auto fun = context.getFunction(node->m_name, tmpTypes);
    if(fun->m_signature.m_returnType.m_size != 0)
        call.m_optReturn = makeOrGetRegister("%return", ictx);

    ictx.m_instructions.push_back(std::make_unique<CallInstruction>(call));
    return makeOrGetRegister("%return", ictx);
}