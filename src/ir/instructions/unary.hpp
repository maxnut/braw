#pragma once

#include "ir/instruction.hpp"
#include "ir/operand.hpp"

struct UnaryInstruction : Instruction {
    UnaryInstruction(Type type) : Instruction(type) {}

    Operand m_operator;
};