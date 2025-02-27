#include "ir_builder.hpp"
#include <memory>

Function IRBuilder::build(const AST::FunctionDefinitionNode* node, BrawContext& context) {
    Function f;
    
    IRFunctionContext ictx;
    
    if(context.getTypeInfo(node->m_signature.m_returnType).value().m_size != 0) {
        if(context.getTypeInfo(node->m_signature.m_returnType).value().m_builtin) {
            if(node->m_signature.m_returnType.m_name == "float" || node->m_signature.m_returnType.m_name == "double")
                f.m_optReturn = makeOrGetRegister("%returnF", ictx);
            else
                f.m_optReturn = makeOrGetRegister("%return", ictx);
        }
        else {
            f.m_optReturn = makeOrGetRegister("%returnPtr", ictx);
            f.m_args.push_back(f.m_optReturn);
        }

        ictx.m_returnRegister = f.m_optReturn;
    }

    for(auto& arg : node->m_signature.m_parameters) {
        f.m_args.push_back(makeOrGetRegister("%" + arg->m_name.m_name + "_0", ictx));
        f.m_args.back()->m_type = context.getTypeInfo(arg->m_type).value();
        f.m_args.back()->m_registerType = getRegisterType(f.m_args.back()->m_type);
    }

    Label label;
    label.m_id = node->m_signature.m_name;
    ictx.m_instructions.push_back(std::make_unique<Label>(label));
    build(node->m_scope.get(), context, ictx);

    if(ictx.m_instructions.back()->m_type != Instruction::Return)
        ictx.m_instructions.push_back(std::make_unique<Instruction>(Instruction::Return));

    f.m_instructions = std::move(ictx.m_instructions);
    f.m_name = node->m_signature.m_name;
    return f;
}