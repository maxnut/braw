#include "ir_builder.hpp"
#include "ir/register.hpp"
#include "parser/nodes/variable_declaration.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "utils.hpp"

void IRBuilder::build(const AST::Node* node, BrawContext& context, IRFunctionContext& ictx) {
    switch(node->m_type) {
        case AST::Node::VariableDeclaration:
            return build((const AST::VariableDeclarationNode*)node, context, ictx);
        case AST::Node::If:
            return build((const AST::IfNode*)node, context, ictx);
        case AST::Node::Return:
            return build((const AST::ReturnNode*)node, context, ictx);
        case AST::Node::Type::BinaryOperator:
            if(static_cast<const AST::BinaryOperatorNode*>(node)->m_operator == "=")
                return buildAssignment(static_cast<const AST::BinaryOperatorNode*>(node), context, ictx);
        default:
            buildExpression(node, context, ictx);
            break;
    }
}

TypeInfo IRBuilder::getOperatorType(Operator op, BrawContext& context, IRFunctionContext& ictx) {
    switch(op.index()) {
        case 1: {
            Register r = std::get<Register>(op);
            return context.getTypeInfo(ictx.getRegisterInfo(r.m_id).m_type).value();
        }
        case 2: {
            std::array<TypeInfo, 7> types = {
                context.getTypeInfo("int").value(), context.getTypeInfo("long").value(), context.getTypeInfo("float").value(), context.getTypeInfo("double").value(),
                context.getTypeInfo("bool").value(), Utils::makePointer(context.getTypeInfo("char").value()), Utils::makePointer(context.getTypeInfo("void").value())
            };
            return types.at(std::get<Value>(op).index());
        }
        case 3:
            return Utils::makePointer(context.getTypeInfo("void").value());
        default:
            break;
    }
    
    return context.getTypeInfo("void").value();
}