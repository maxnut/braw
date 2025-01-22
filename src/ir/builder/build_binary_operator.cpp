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

    Register target{"%" + std::to_string((uintptr_t)node)};

    if(node->m_operator == "+") {
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, target, left));
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::Add, target, right));
    }
    else if(node->m_operator == "*") {
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, target, left));
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::Multiply, target, right));
    }
    else if(node->m_operator == "-") {
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, target, left));
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::Subtract, target, right));
    }
    else if(node->m_operator == "==") {
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, target, left));
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::CompareEquals, target, right));
    }
    else if(node->m_operator == "!=") {
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, target, left));
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::CompareNotEquals, target, right));
    }
    else if(node->m_operator == ">") {
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, target, right));
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::CompareLessEquals, target, left));
    }
    else if(node->m_operator == "<") {
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, target, right));
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::CompareGreaterEquals, target, left));
    }
    else if(node->m_operator == "<=") {
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, target, left));
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::CompareLessEquals, target, right));
    }
    else if(node->m_operator == ">=") {
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, target, left));
        ictx.m_instructions.push_back(std::make_unique<BinaryInstruction>(Instruction::CompareGreaterEquals, target, right));
    }
    

    return target;
}