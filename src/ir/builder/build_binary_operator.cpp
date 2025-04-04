#include "ir/instructions/basic.hpp"
#include "ir/register.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "rules.hpp"
#include <cstdint>
#include <memory>
#include <string>

Operand IRBuilder::buildBinaryOperator(const AST::BinaryOperatorNode* node, BrawContext& context, IRFunctionContext& ictx) {
    if(node->m_operator == "||") {
        std::string name = "%" + std::to_string((uintptr_t)node);
        std::shared_ptr<Register> target = makeOrGetRegister(name, ictx);
        Label label{node->m_right->m_rangeEnd};
        label.m_id = "." + std::to_string((uintptr_t)node);
        Operand left = buildExpression(node->m_left.get(), context, ictx);
        moveToRegister(name, left, node->m_rangeBegin, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::JumpTrue, node->m_right->m_rangeEnd, left, label));
        Operand right = buildExpression(node->m_right.get(), context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Or, node->m_rangeBegin, target, right));
        ictx.m_instructions.push_back(std::make_unique<Label>(label));
        return target;
    }

    Operand left = buildExpression(node->m_left.get(), context, ictx);
    Operand right = buildExpression(node->m_right.get(), context, ictx);

    std::string name = "%" + std::to_string((uintptr_t)node);
    std::shared_ptr<Register> target = makeOrGetRegister(name, ictx);

    if(node->m_operator == "+") {
        moveToRegister(name, left, node->m_rangeBegin, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Add, node->m_rangeBegin, target, right));
    }
    else if(node->m_operator == "*") {
        moveToRegister(name, left, node->m_rangeBegin, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Multiply, node->m_rangeBegin, target, right));
    }
    else if(node->m_operator == "/") {
        moveToRegister(name, left, node->m_rangeBegin, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Divide, node->m_rangeBegin, target, right));
    }
    else if(node->m_operator == "%") {
        moveToRegister(name, left, node->m_rangeBegin, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Modulo, node->m_rangeBegin, target, right));
    }
    else if(node->m_operator == "-") {
        moveToRegister(name, left, node->m_rangeBegin, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Subtract, node->m_rangeBegin, target, right));
    }
    else if(node->m_operator == "==") {
        moveToRegister(name, left, node->m_rangeBegin, context, ictx);
        auto newTarget = makeOrGetRegister(name + "_0", ictx);
        newTarget->m_type = context.getTypeInfo(getOperandType(left, context, ictx).m_operators["=="].m_returnType).value();
        newTarget->m_registerType = getRegisterType(newTarget->m_type);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::CompareEquals, node->m_rangeBegin, target, right, newTarget));
        target = newTarget;
    }
    else if(node->m_operator == "!=") {
        moveToRegister(name, left, node->m_rangeBegin, context, ictx);
        auto newTarget = makeOrGetRegister(name + "_0", ictx);
        newTarget->m_type = context.getTypeInfo(getOperandType(left, context, ictx).m_operators["!="].m_returnType).value();
        newTarget->m_registerType = getRegisterType(newTarget->m_type);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::CompareNotEquals,node->m_rangeBegin,  target, right, newTarget));
        target = newTarget;
    }
    else if(node->m_operator == ">") {
        moveToRegister(name, left, node->m_rangeBegin, context, ictx);
        auto newTarget = makeOrGetRegister(name + "_0", ictx);
        newTarget->m_type = context.getTypeInfo(getOperandType(left, context, ictx).m_operators[">"].m_returnType).value();
        newTarget->m_registerType = getRegisterType(newTarget->m_type);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::CompareGreater, node->m_rangeBegin, target, right, newTarget));
        target = newTarget;
    }
    else if(node->m_operator == "<") {
        moveToRegister(name, left, node->m_rangeBegin, context, ictx);
        auto newTarget = makeOrGetRegister(name + "_0", ictx);
        newTarget->m_type = context.getTypeInfo(getOperandType(left, context, ictx).m_operators["<"].m_returnType).value();
        newTarget->m_registerType = getRegisterType(newTarget->m_type);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::CompareLess, node->m_rangeBegin, target, right, newTarget));
        target = newTarget;
    }
    else if(node->m_operator == "<=") {
        moveToRegister(name, left, node->m_rangeBegin, context, ictx);
        auto newTarget = makeOrGetRegister(name + "_0", ictx);
        newTarget->m_type = context.getTypeInfo(getOperandType(left, context, ictx).m_operators["<="].m_returnType).value();
        newTarget->m_registerType = getRegisterType(newTarget->m_type);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::CompareLessEquals, node->m_rangeBegin, target, right, newTarget));
        target = newTarget;
    }
    else if(node->m_operator == ">=") {
        moveToRegister(name, left, node->m_rangeBegin, context, ictx);
        auto newTarget = makeOrGetRegister(name + "_0", ictx);
        newTarget->m_type = context.getTypeInfo(getOperandType(left, context, ictx).m_operators[">="].m_returnType).value();
        newTarget->m_registerType = getRegisterType(newTarget->m_type);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::CompareGreaterEquals, node->m_rangeBegin, target, right, newTarget));
        target = newTarget;
    }
    else if(node->m_operator == "&" || node->m_operator == "&&") {
        moveToRegister(name, left, node->m_rangeBegin, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::And, node->m_rangeBegin, target, right));
    }
    else if(node->m_operator == "|") {
        moveToRegister(name, left, node->m_rangeBegin, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Or, node->m_rangeBegin, target, right));
    }
    else if(node->m_operator == "^") {
        moveToRegister(name, left, node->m_rangeBegin, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Xor, node->m_rangeBegin, target, right));
    }
    
    return target;
}