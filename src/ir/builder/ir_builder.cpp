#include "ir_builder.hpp"
#include "ir/instruction.hpp"
#include "ir/instructions/basic.hpp"
#include "ir/register.hpp"
#include "parser/nodes/variable_declaration.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "rules.hpp"
#include "utils.hpp"

#include <array>
#include <memory>

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
        case AST::Node::Scope:
            return build((const AST::ScopeNode*)node, context, ictx);
        default:
            buildExpression(node, context, ictx);
            break;
    }
}

TypeInfo IRBuilder::getOperandType(Operand op, BrawContext& context, IRFunctionContext& ictx) {
    switch(op.index()) {
        case 1: {
            return std::get<std::shared_ptr<Register>>(op)->m_type;
        }
        case 2: {
            std::array<TypeInfo, 7> types = {
                context.getTypeInfo("int").value(), context.getTypeInfo("long").value(), context.getTypeInfo("float").value(), context.getTypeInfo("double").value(),
                context.getTypeInfo("bool").value(), Utils::makePointer(context.getTypeInfo("char").value()), Utils::makePointer(context.getTypeInfo("void").value())
            };
            return types.at(std::get<Value>(op).index());
        }
        case 3: {
            auto addr = std::get<Address>(op);
            if(Rules::isPtr(addr.m_base->m_type.m_name))
                return Utils::getRawType(addr.m_base->m_type, context).value();
            auto off = addr.m_offset >= 0 ? addr.m_offset : addr.m_base->m_type.m_size + addr.m_offset;
            return context.getTypeInfo(addr.m_base->m_type.memberByOffset(off).value().m_type).value();
            // return Utils::makePointer(context.getTypeInfo("void").value());
        }
        default:
            break;
    }
    
    return context.getTypeInfo("void").value();
}

RegisterType IRBuilder::getRegisterType(const TypeInfo& type) {
    if(type.m_name == "int")
        return RegisterType::Signed;
    else if(type.m_name == "long")
        return RegisterType::Signed;
    else if(type.m_name == "float")
        return RegisterType::Single;
    else if(type.m_name == "double")
        return RegisterType::Double;
    else if(type.m_name == "char")
        return RegisterType::Signed;
    else if(type.m_name == "bool")
        return RegisterType::Signed;
    else if(Rules::isPtr(type.m_name)) 
        return RegisterType::Pointer;

    return RegisterType::Struct;
}

void IRBuilder::moveToRegister(const std::string& name, Operand& op, BrawContext& context, IRFunctionContext& ictx) {
    std::shared_ptr<Register> reg = makeOrGetRegister(name, ictx);

    if(reg->m_type.m_name == "")
        reg->m_type = getOperandType(op, context, ictx);
    reg->m_registerType = getRegisterType(reg->m_type);
    Instruction::Type instrType = reg->m_registerType == RegisterType::Struct ? Instruction::Copy : reg->m_registerType == RegisterType::Pointer ? Instruction::Point : Instruction::Move;
    ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(instrType, reg, op));
}

std::shared_ptr<Register> IRBuilder::makeOrGetRegister(const std::string& name, IRFunctionContext& ictx) {
    if(ictx.m_registers.contains(name))
        return ictx.m_registers[name];

    std::shared_ptr<Register> reg = std::make_shared<Register>(name);
    ictx.m_registers[name] = reg;
    return reg;
}