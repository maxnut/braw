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
    int64_t offset = t.m_members.at(node->m_data).m_offset; 
    auto tMember = context.getTypeInfo(t.m_members.at(node->m_data).m_type).value();
    switch(op.index()) {
        default:
            return Address(std::get<std::shared_ptr<Register>>(op), offset, tMember);
        case 3: {
            Address addr = std::get<Address>(op);
            return Address(addr.m_base, addr.m_offset + offset, tMember);
        }
    }
}

Operand IRBuilder::dereferenceOperator(const AST::UnaryOperatorNode* node, const Operand& op, BrawContext& context, IRFunctionContext& ictx) {
    auto tmp = op;
    Operand ret;
    ret = makeOrGetRegister("%" + std::to_string((uintptr_t)node), ictx);
    ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Dereference, ret, tmp));
    auto retReg = std::get<std::shared_ptr<Register>>(ret);
    retReg->m_type = Utils::getRawType(getOperandType(op, context, ictx), context).value();
    retReg->m_registerType = getRegisterType(retReg->m_type);
    return ret;
}

Operand IRBuilder::addressOperator(const AST::UnaryOperatorNode* node, const Operand& op, BrawContext& context, IRFunctionContext& ictx) {
    Operand ret;
    ret = makeOrGetRegister("%" + std::to_string((uintptr_t)node), ictx);
    ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Point, ret, op));
    auto retReg = std::get<std::shared_ptr<Register>>(ret);
    retReg->m_type = Utils::makePointer(getOperandType(op, context, ictx));
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
        Operand index = buildExpression(node->m_expression.get(), context, ictx);
        if(index.index() != 1) {
            auto tmp = makeOrGetRegister("%" + std::to_string((uintptr_t)node) + "_0", ictx);
            tmp->m_type = getOperandType(index, context, ictx);
            tmp->m_registerType = getRegisterType(tmp->m_type);
            moveToRegister(tmp->m_id, index, context, ictx);
            index = tmp;
        }
        if(op.index() != 1) {
            auto tmp = makeOrGetRegister("%" + std::to_string((uintptr_t)node) + "_1", ictx);
            tmp->m_type = getOperandType(op, context, ictx);
            tmp->m_registerType = getRegisterType(tmp->m_type);
            moveToRegister(tmp->m_id, op, context, ictx);
            op = tmp;
        }

        if(std::get<1>(index)->m_type.m_name == INT_T) {
            auto tmp = makeOrGetRegister(std::get<1>(index)->m_id + "_0", ictx);
            upsize(index, tmp, context.getTypeInfo(LONG_T).value(), context, ictx);
            index = tmp;
        }
        
        TypeInfo raw = Utils::getRawType(getOperandType(op, context, ictx), context).value();
        ret = Address(std::get<1>(op), 0, raw, std::get<1>(index), raw.m_size);
    }
    else if(node->m_operator == "cast") {
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
                    upsize(op, std::get<1>(ret), context.getTypeInfo(node->m_data).value(), context, ictx);
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
                    upsize(op, std::get<1>(ret), context.getTypeInfo(node->m_data).value(), context, ictx);
                }
            }
        }
    }

    return ret;
}