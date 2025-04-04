#include "ir/address.hpp"
#include "ir/instructions/basic.hpp"
#include "ir/operand.hpp"
#include "ir/register.hpp"
#include "ir/value.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "rules.hpp"
#include "type_info.hpp"
#include "utils.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <variant>

Operand IRBuilder::dotOperator(const AST::UnaryOperatorNode* node, const Operand& op, BrawContext& context, IRFunctionContext& ictx) {
    auto t = getOperandType(op, context, ictx);
    MemberInfo member = t.m_members.at(node->m_data);
    int64_t offset = member.m_offset; 
    auto tMember = context.getTypeInfo(member.m_type).value();
    switch(op.index()) {
        default:
            return Address(std::get<std::shared_ptr<Register>>(op), offset, tMember, nullptr, 0, member.m_scale);
        case 3: {
            Address addr = std::get<Address>(op);
            return Address(addr.m_base, addr.m_offset + offset, tMember, nullptr, 0, member.m_scale);
        }
    }
}

Operand IRBuilder::dereferenceOperator(const AST::UnaryOperatorNode* node, const Operand& op, BrawContext& context, IRFunctionContext& ictx) {
    auto tmp = op;
    Operand ret;
    ret = makeOrGetRegister("%" + std::to_string((uintptr_t)node), ictx);
    ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Dereference, node->m_rangeBegin, ret, tmp));
    auto retReg = std::get<std::shared_ptr<Register>>(ret);
    retReg->m_type = Utils::getRawType(getOperandType(op, context, ictx), context).value();
    retReg->m_registerType = getRegisterType(retReg->m_type);
    return ret;
}

Operand IRBuilder::addressOperator(const AST::UnaryOperatorNode* node, const Operand& op, BrawContext& context, IRFunctionContext& ictx) {
    Operand ret;
    ret = makeOrGetRegister("%" + std::to_string((uintptr_t)node), ictx);
    ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Point, node->m_rangeBegin, ret, op));
    auto retReg = std::get<std::shared_ptr<Register>>(ret);
    retReg->m_type = Utils::makePointer(getOperandType(op, context, ictx));
    retReg->m_registerType = getRegisterType(retReg->m_type);
    return ret;
}

Operand IRBuilder::subscriptOperator(const AST::UnaryOperatorNode* node, Operand& op, BrawContext& context, IRFunctionContext& ictx) {
    Operand index = buildExpression(node->m_expression.get(), context, ictx);
    if(index.index() != 1) {
        auto tmp = makeOrGetRegister("%" + std::to_string((uintptr_t)node) + "_0", ictx);
        tmp->m_type = getOperandType(index, context, ictx);
        tmp->m_registerType = getRegisterType(tmp->m_type);
        moveToRegister(tmp->m_id, index, node->m_rangeBegin, context, ictx);
        index = tmp;
    }
    if(op.index() != 1) {
        auto tmp = makeOrGetRegister("%" + std::to_string((uintptr_t)node) + "_1", ictx);
        tmp->m_type = getOperandType(op, context, ictx);
        tmp->m_registerType = getRegisterType(tmp->m_type);
        if(std::holds_alternative<Address>(op) && std::get<Address>(op).m_scaleSize > 1)
            ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Point, node->m_rangeBegin, tmp, op));
        else
            moveToRegister(tmp->m_id, op, node->m_rangeBegin, context, ictx);
        op = tmp;
    }

    if(std::get<1>(index)->m_type.m_name == INT_T) {
        auto tmp = makeOrGetRegister(std::get<1>(index)->m_id + "_0", ictx);
        upsize(index, tmp, node->m_expression->m_rangeBegin, context.getTypeInfo(LONG_T).value(), context, ictx);
        index = tmp;
    }
    
    TypeInfo raw = Utils::getRawType(getOperandType(op, context, ictx), context).value();
    return Address(std::get<1>(op), 0, raw, std::get<1>(index), raw.m_size);
}

Operand IRBuilder::castOperator(const AST::UnaryOperatorNode* node, Operand& op, BrawContext& context, IRFunctionContext& ictx) {
    Operand ret;
    TypeInfo opType = getOperandType(op, context, ictx);
    if(Rules::isPtr(node->m_data) || node->m_data.m_name == LONG_T) {
        if(Rules::isPtr(opType.m_name) || opType.m_name == LONG_T)
            ret = op;
        else if(opType.m_name == INT_T || opType.m_name == CHAR_T) {
            if(std::holds_alternative<Value>(op)) {
                ret = op;
                if(opType.m_name == INT_T)
                    ret = (long)std::get<int>(std::get<Value>(ret));
                else
                    ret = (long)std::get<char>(std::get<Value>(ret));
            }
            else {
                ret = makeOrGetRegister("%" + std::to_string((uintptr_t)node), ictx);
                upsize(op, std::get<1>(ret), node->m_rangeBegin, context.getTypeInfo(node->m_data).value(), context, ictx);
            }
        }
    }
    else if(node->m_data.m_name == INT_T) {
        if(opType.m_name == CHAR_T) {
            if(std::holds_alternative<Value>(op)) {
                ret = op;
                ret = (int)std::get<char>(std::get<Value>(ret));
            }
            else {
                ret = makeOrGetRegister("%" + std::to_string((uintptr_t)node), ictx);
                upsize(op, std::get<1>(ret), node->m_rangeBegin, context.getTypeInfo(node->m_data).value(), context, ictx);
            }
        }
        else if(opType.m_name == LONG_T) {
            if(std::holds_alternative<Value>(op)) {
                ret = op;
                ret = (int)std::get<long>(std::get<Value>(ret));
            }
            else {
                ret = makeOrGetRegister("%" + std::to_string((uintptr_t)node), ictx);
                downsize(op, std::get<1>(ret), node->m_rangeBegin, context.getTypeInfo(node->m_data).value(), context, ictx);
            }
        }
    }
    else if(node->m_data.m_name == CHAR_T) {
        if(opType.m_name == INT_T) {
            if(std::holds_alternative<Value>(op)) {
                ret = op;
                ret = (char)std::get<int>(std::get<Value>(ret));
            }
            else {
                ret = makeOrGetRegister("%" + std::to_string((uintptr_t)node), ictx);
                downsize(op, std::get<1>(ret), node->m_rangeBegin, context.getTypeInfo(node->m_data).value(), context, ictx);
            }
        }
        else if(opType.m_name == LONG_T) {
            if(std::holds_alternative<Value>(op)) {
                ret = op;
                ret = (char)std::get<long>(std::get<Value>(ret));
            }
            else {
                ret = makeOrGetRegister("%" + std::to_string((uintptr_t)node), ictx);
                downsize(op, std::get<1>(ret), node->m_rangeBegin, context.getTypeInfo(node->m_data).value(), context, ictx);
            }
        }
    }
    if(ret.index() == 1) {
        std::shared_ptr<Register> reg = std::get<1>(ret);
        std::shared_ptr<Register> clone = std::make_shared<Register>(reg->m_id, reg->m_type, reg->m_registerType, reg->m_scale);
        clone->m_type = context.getTypeInfo(node->m_data).value();
        clone->m_registerType = getRegisterType(reg->m_type);
        ret = clone;
    }
    else if(ret.index() == 3) {
        auto add = std::get<3>(ret);
        add.m_typeInfo = context.getTypeInfo(node->m_data).value();
        ret = add;
    }
    return ret;
}

Operand IRBuilder::logicalNotOperator(const AST::UnaryOperatorNode* node, Operand& op, BrawContext& context, IRFunctionContext& ictx) {
    Operand ret = makeOrGetRegister("%" + std::to_string((uintptr_t)node), ictx);
    ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::LogicalNot, node->m_rangeBegin, ret, op));
    auto retReg = std::get<std::shared_ptr<Register>>(ret);
    retReg->m_type = context.getTypeInfo(BOOL_T).value();
    retReg->m_registerType = getRegisterType(retReg->m_type);
    return ret;
}

Operand IRBuilder::buildUnaryOperator(const AST::UnaryOperatorNode* node, BrawContext& context, IRFunctionContext& ictx) {
    Operand ret;
    Operand op = buildExpression(node->m_operand.get(), context, ictx);

    if(node->m_operator == ".") {
        ret = dotOperator(node, op, context, ictx);
    }
    else if(node->m_operator == "&") 
        ret = addressOperator(node, op, context, ictx);
    else if(node->m_operator == "*") 
        ret = dereferenceOperator(node, op, context, ictx);
    else if(node->m_operator == "->") {
        ret = dereferenceOperator(node, op, context, ictx);
        ret = dotOperator(node, ret, context, ictx);
    }
    else if(node->m_operator == "[]") {
        ret = subscriptOperator(node, op, context, ictx);
    }
    else if(node->m_operator == "cast") {
        ret = castOperator(node, op, context, ictx);
    }
    else if(node->m_operator == "!") {
        ret = logicalNotOperator(node, op, context, ictx);
    }

    return ret;
}