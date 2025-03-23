#include "ir_builder.hpp"
#include "ir/instruction.hpp"
#include "ir/instructions/basic.hpp"
#include "ir/operand.hpp"
#include "ir/register.hpp"
#include "parser/nodes/for.hpp"
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
        case AST::Node::While:
            return build((const AST::WhileNode*)node, context, ictx);
        case AST::Node::For:
            return build((const AST::ForNode*)node, context, ictx);
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
                context.getTypeInfo(INT_T).value(), context.getTypeInfo(LONG_T).value(), context.getTypeInfo(FLOAT_T).value(), context.getTypeInfo(DOUBLE_T).value(),
                context.getTypeInfo(BOOL_T).value(), Utils::makePointer(context.getTypeInfo(CHAR_T).value()), Utils::makePointer(context.getTypeInfo(VOID_T).value())
            };
            return types.at(std::get<Value>(op).index());
        }
        case 3: {
            auto addr = std::get<Address>(op);
            return addr.m_typeInfo;
        }
        default:
            break;
    }
    
    return context.getTypeInfo(VOID_T).value();
}

RegisterType IRBuilder::getRegisterType(const TypeInfo& type) {
    if(type.m_name == INT_T)
        return RegisterType::Signed;
    else if(type.m_name == LONG_T)
        return RegisterType::Signed;
    else if(type.m_name == FLOAT_T)
        return RegisterType::Single;
    else if(type.m_name == DOUBLE_T)
        return RegisterType::Double;
    else if(type.m_name == CHAR_T)
        return RegisterType::Signed;
    else if(type.m_name == BOOL_T)
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
    Instruction::Type instrType = reg->m_registerType == RegisterType::Struct ? Instruction::Copy : Instruction::Move;
    ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(instrType, reg, op));
}

std::shared_ptr<Register> IRBuilder::makeOrGetRegister(const std::string& name, IRFunctionContext& ictx) {
    if(ictx.m_registers.contains(name))
        return ictx.m_registers[name];

    std::shared_ptr<Register> reg = std::make_shared<Register>(name);
    ictx.m_registers[name] = reg;
    return reg;
}

void IRBuilder::upsize(Operand& op, std::shared_ptr<Register> to, const TypeInfo& toType, BrawContext& context, IRFunctionContext& ictx) {
    if(op.index() == 1 && std::get<1>(op) == to) {
        auto reg = std::get<1>(op);
        to = std::make_shared<Register>();
        to->m_id = reg->m_id;
        to->m_scale = reg->m_scale;
    }

    to->m_type = toType;
    to->m_registerType = getRegisterType(to->m_type);
    ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Upsize, to, op));
}