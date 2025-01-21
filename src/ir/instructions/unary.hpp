#pragma once

#include "ir/instruction.hpp"
#include "ir/operator.hpp"

struct UnaryInstruction : Instruction {
    UnaryInstruction(Type type) : Instruction(type) {}

    Operator m_operator;
};