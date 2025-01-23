#include "ir_builder.hpp"

Function IRBuilder::build(const AST::FunctionDefinitionNode* node, BrawContext& context) {
    Function f;

    if(context.getTypeInfo(node->m_signature.m_returnType).value().m_size != 0)
        f.m_optReturn = Register{"%return"};

    IRFunctionContext ictx;
    ictx.m_tables.push_back(RegisterTable());

    for(auto& arg : node->m_signature.m_parameters) {
        f.m_args.push_back(Register{"%" + arg->m_name.m_name});

        ictx.m_tables.back().insert({
            "%" + arg->m_name.m_name,
            {arg->m_type}
        });
    }

    Label label;
    label.m_id = node->m_signature.m_name;
    ictx.m_instructions.push_back(std::make_unique<Label>(label));
    build(node->m_scope.get(), context, ictx);

    if(ictx.m_instructions.back()->m_type != Instruction::Return)
        ictx.m_instructions.push_back(std::make_unique<Instruction>(Instruction::Return));

    f.m_instructions = std::move(ictx.m_instructions);
    return f;
}