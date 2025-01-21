#pragma once

#include "ir/instruction.hpp"
#include "ir/operator.hpp"

struct BinaryInstruction : Instruction {
    BinaryInstruction(Type type) : Instruction(type) {}
    BinaryInstruction(Type type, Operator l, Operator r) : Instruction(type), m_left(l), m_right(r) {}

    Operator m_left;
    Operator m_right;
};