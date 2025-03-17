#include "ir/instructions/basic.hpp"
#include "ir/register.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "rules.hpp"
#include <cstdint>
#include <memory>
#include <string>

Operand IRBuilder::buildBinaryOperator(const AST::BinaryOperatorNode* node, BrawContext& context, IRFunctionContext& ictx) {
    Operand left = buildExpression(node->m_left.get(), context, ictx);
    Operand right = buildExpression(node->m_right.get(), context, ictx);

    std::string name = "%" + std::to_string((uintptr_t)node);
    std::shared_ptr<Register> target = makeOrGetRegister(name, ictx);

    if(Rules::isPtr(getOperandType(left, context, ictx).m_name) && getOperandType(right, context, ictx).m_name == INT_T) {
        auto tmp = makeOrGetRegister(std::get<1>(right)->m_id + "_tmp", ictx);
        upsize(std::get<1>(right), tmp, context, ictx);   
        right = tmp;
    }
    else if(Rules::isPtr(getOperandType(right, context, ictx).m_name) && getOperandType(left, context, ictx).m_name == INT_T) {
        auto tmp = makeOrGetRegister(std::get<1>(left)->m_id + "_tmp", ictx);
        upsize(std::get<1>(left), tmp, context, ictx);   
        left = tmp;
    }

    if(node->m_operator == "+") {
        moveToRegister(name, left, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Add, target, right));
    }
    else if(node->m_operator == "*") {
        moveToRegister(name, left, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Multiply, target, right));
    }
    else if(node->m_operator == "-") {
        moveToRegister(name, left, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Subtract, target, right));
    }
    else if(node->m_operator == "==") {
        moveToRegister(name, left, context, ictx);
        auto newTarget = makeOrGetRegister(name + "_0", ictx);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::CompareEquals, target, right, newTarget));
        target = newTarget;
    }
    else if(node->m_operator == "!=") {
        moveToRegister(name, left, context, ictx);
        auto newTarget = makeOrGetRegister(name + "_0", ictx);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::CompareNotEquals, target, right, newTarget));
        target = newTarget;
    }
    else if(node->m_operator == ">") {
        moveToRegister(name, left, context, ictx);
        auto newTarget = makeOrGetRegister(name + "_0", ictx);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::CompareGreater, target, right, newTarget));
        target = newTarget;
    }
    else if(node->m_operator == "<") {
        moveToRegister(name, left, context, ictx);
        auto newTarget = makeOrGetRegister(name + "_0", ictx);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::CompareLess, target, right, newTarget));
        target = newTarget;
    }
    else if(node->m_operator == "<=") {
        moveToRegister(name, left, context, ictx);
        auto newTarget = makeOrGetRegister(name + "_0", ictx);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::CompareLessEquals, target, right, newTarget));
        target = newTarget;
    }
    else if(node->m_operator == ">=") {
        moveToRegister(name, left, context, ictx);
        auto newTarget = makeOrGetRegister(name + "_0", ictx);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::CompareGreaterEquals, target, right, newTarget));
        target = newTarget;
    }
    
    return target;
}