#pragma once

#include "ir/instruction.hpp"
#include "ir/operand.hpp"
#include <variant>

struct BasicInstruction : Instruction {
    BasicInstruction(Type type, Operand o1 = std::monostate{}, Operand o2 = std::monostate{}, Operand o3 = std::monostate{}, Operand o4 = std::monostate{}) : Instruction(type), m_o1(o1), m_o2(o2), m_o3(o3), m_o4(o4) {}

    Operand m_o1;
    Operand m_o2;
    Operand m_o3;
    Operand m_o4;
};