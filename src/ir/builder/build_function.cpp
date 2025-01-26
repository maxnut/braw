#include "ir_builder.hpp"
#include <memory>

Function IRBuilder::build(const AST::FunctionDefinitionNode* node, BrawContext& context) {
    Function f;
    
    IRFunctionContext ictx;
    
    if(context.getTypeInfo(node->m_signature.m_returnType).value().m_size != 0)
        f.m_optReturn = makeOrGetRegister("%return", ictx);

    for(auto& arg : node->m_signature.m_parameters) {
        f.m_args.push_back(makeOrGetRegister("%" + arg->m_name.m_name + "_0", ictx));
        f.m_args.back()->m_type = context.getTypeInfo(arg->m_type).value();
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