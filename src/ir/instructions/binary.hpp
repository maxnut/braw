#pragma once

#include "ir/instruction.hpp"
#include "ir/operator.hpp"

struct BinaryInstruction : Instruction {
    BinaryInstruction(Type type) : Instruction(type) {}

    Operator m_left;
    Operator m_right;
};