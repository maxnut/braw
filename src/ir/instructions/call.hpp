#pragma once

#include "ir/instruction.hpp"
#include "ir/operator.hpp"
#include "ir/register.hpp"
#include <vector>

struct CallInstruction : Instruction {
    CallInstruction() : Instruction(Type::Call) {}

    std::string m_id;
    Register m_optReturn;
    std::vector<Operator> m_parameters;
};