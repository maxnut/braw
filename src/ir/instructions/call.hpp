#pragma once

#include "ir/instruction.hpp"
#include "ir/operator.hpp"
#include <vector>

struct CallInstruction : Instruction {
    CallInstruction() : Instruction(Type::Call) {}

    std::string m_id;
    std::vector<Operator> m_parameters;
};