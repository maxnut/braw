#include "ir/address.hpp"
#include "ir/instructions/basic.hpp"
#include "ir/operand.hpp"
#include "ir/register.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "utils.hpp"

#include <memory>
#include <string>

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

    return ret;
}