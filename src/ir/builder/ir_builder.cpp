#include "ir_builder.hpp"
#include "parser/nodes/variable_declaration.hpp"

void IRBuilder::build(const AST::Node* node, BrawContext& context, Instructions& is) {
    switch(node->m_type) {
        case AST::Node::VariableDeclaration:
            return build((const AST::VariableDeclarationNode*)node, context, is);
        case AST::Node::If:
            return build((const AST::IfNode*)node, context, is);
        case AST::Node::Return:
            return build((const AST::ReturnNode*)node, context, is);
        default:
            break;
    }
}