#include "ir/instructions/binary.hpp"
#include "ir/register.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/binary_operator.hpp"
#include <cstdint>
#include <memory>
#include <string>

Operator IRBuilder::buildBinaryOperator(const AST::BinaryOperatorNode* node, BrawContext& context, Instructions& is) {
    Operator left = buildExpression(node->m_left.get(), context, is);
    Operator right = buildExpression(node->m_right.get(), context, is);

    Register target{"%" + std::to_string((uintptr_t)node)};

    if(node->m_operator == "+") {
        is.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, target, left));
        is.push_back(std::make_unique<BinaryInstruction>(Instruction::Add, target, right));
    }
    else if(node->m_operator == "*") {
        is.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, target, left));
        is.push_back(std::make_unique<BinaryInstruction>(Instruction::Multiply, target, right));
    }
    else if(node->m_operator == "-") {
        is.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, target, left));
        is.push_back(std::make_unique<BinaryInstruction>(Instruction::Subtract, target, right));
    }
    else if(node->m_operator == "==") {
        is.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, target, left));
        is.push_back(std::make_unique<BinaryInstruction>(Instruction::CompareEquals, target, right));
    }
    else if(node->m_operator == "!=") {
        is.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, target, left));
        is.push_back(std::make_unique<BinaryInstruction>(Instruction::CompareNotEquals, target, right));
    }
    else if(node->m_operator == ">") {
        is.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, target, right));
        is.push_back(std::make_unique<BinaryInstruction>(Instruction::CompareLessEquals, target, left));
    }
    else if(node->m_operator == "<") {
        is.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, target, right));
        is.push_back(std::make_unique<BinaryInstruction>(Instruction::CompareGreaterEquals, target, left));
    }
    else if(node->m_operator == "<=") {
        is.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, target, left));
        is.push_back(std::make_unique<BinaryInstruction>(Instruction::CompareLessEquals, target, right));
    }
    else if(node->m_operator == ">=") {
        is.push_back(std::make_unique<BinaryInstruction>(Instruction::Move, target, left));
        is.push_back(std::make_unique<BinaryInstruction>(Instruction::CompareGreaterEquals, target, right));
    }
    

    return target;
}