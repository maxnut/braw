#pragma once

#include "ir/instruction.hpp"
#include "ir/operand.hpp"

struct BinaryInstruction : Instruction {
    BinaryInstruction(Type type) : Instruction(type) {}
    BinaryInstruction(Type type, Operand l, Operand r) : Instruction(type), m_left(l), m_right(r) {}

    Operand m_left;
    Operand m_right;
};