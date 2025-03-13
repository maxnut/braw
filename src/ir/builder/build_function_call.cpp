#include "ir_builder.hpp"
#include "ir/instructions/call.hpp"
#include "parser/nodes/function_call.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "rules.hpp"

Operand IRBuilder::buildCall(const AST::FunctionCallNode* node, BrawContext& context, IRFunctionContext& ictx) {
    CallInstruction call;
    call.m_id = node->m_name;

    std::vector<TypeInfo> tmpTypes;

    for(auto& param : node->m_parameters) {
        Operand op = buildExpression(param.get(), context, ictx);

        TypeInfo t = getOperandType(op, context, ictx);
        if(op.index() == 1 && t.m_builtin) {
            for(size_t i = 0; i < ictx.m_function->m_args.size(); i++) {
                auto& arg = ictx.m_function->m_args[i];
                if(arg->m_id == std::get<1>(op)->m_id) {
                    auto newReg = makeOrGetRegister(arg->m_id + "_" + std::to_string(i), ictx);
                    newReg->m_type = arg->m_type;
                    newReg->m_registerType = arg->m_registerType;
                    moveToRegister(newReg->m_id, op, context, ictx);
                    op = newReg;
                    break;
                }
            }
        }

        if(param->m_type == AST::Node::UnaryOperator && static_cast<const AST::UnaryOperatorNode*>(param.get())->m_operator == "cast")
            t = context.getTypeInfo(static_cast<const AST::UnaryOperatorNode*>(param.get())->m_data).value();
        call.m_parameters.push_back(op);
        tmpTypes.push_back(t);
    }

    std::string name = "%" + std::to_string((uintptr_t)node);

    auto fun = context.getFunction(node->m_name, tmpTypes);
    call.m_returnType = fun->m_returnType;
    if(fun->m_returnType.m_size != 0) {
        call.m_optReturn = makeOrGetRegister(name, ictx);
        call.m_optReturn->m_registerType = getRegisterType(fun->m_returnType);
        call.m_optReturn->m_type = fun->m_returnType;
    }

    ictx.m_instructions.push_back(std::make_unique<CallInstruction>(call));
    return makeOrGetRegister(name, ictx);
}