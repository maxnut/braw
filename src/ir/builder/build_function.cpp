#include "ir_builder.hpp"

Function IRBuilder::build(const AST::FunctionDefinitionNode* node, BrawContext& context) {
    Function f;

    if(context.getTypeInfo(node->m_signature.m_returnType).value().m_size != 0)
        f.m_optReturn = Register{"%return"};

    for(auto& arg : node->m_signature.m_parameters)
        f.m_args.push_back(Register{arg->m_name});

    Label label;
    label.m_id = node->m_signature.m_name;
    f.m_instructions.push_back(std::make_unique<Label>(label));
    build(node->m_scope.get(), context, f.m_instructions);
    return f;
}