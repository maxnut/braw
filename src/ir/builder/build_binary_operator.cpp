#include "ir/instructions/binary.hpp"
#include "ir/register.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/binary_operator.hpp"
#include <cstdint>
#include <memory>
#include <string>

Operator IRBuilder::buildBinaryOperator(const AST::BinaryOperatorNode* node, BrawContext& context, IRFunctionContext& ictx) {
    Operator left = buildExpression(node->m_left.get(), context, ictx);
    Operator right = buildExpression(node->m_right.get(), context, ictx);

    std::string name = "%" + std::to_string((uintptr_t)node);
    std::shared_ptr<Register> target = makeOrGetRegister(name, ictx);

    if(node->m_operator == "+") {
        moveToRegister(name, left, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::Add, target, right));
    }
    else if(node->m_operator == "*") {
        moveToRegister(name, left, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::Multiply, target, right));
    }
    else if(node->m_operator == "-") {
        moveToRegister(name, left, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::Subtract, target, right));
    }
    else if(node->m_operator == "==") {
        moveToRegister(name, left, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::CompareEquals, target, right));
    }
    else if(node->m_operator == "!=") {
        moveToRegister(name, left, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::CompareNotEquals, target, right));
    }
    else if(node->m_operator == ">") {
        moveToRegister(name, right, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::CompareLessEquals, target, left));
    }
    else if(node->m_operator == "<") {
        moveToRegister(name, right, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::CompareGreaterEquals, target, left));
    }
    else if(node->m_operator == "<=") {
        moveToRegister(name, left, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::CompareLessEquals, target, right));
    }
    else if(node->m_operator == ">=") {
        moveToRegister(name, left, context, ictx);
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::CompareGreaterEquals, target, right));
    }
    
    return target;
}