#pragma once

#include "register.hpp"
#include "instruction.hpp"

#include <vector>
#include <memory>

struct Function {
    std::vector<Register> m_args;
    Register m_optReturn;
    std::vector<std::unique_ptr<Instruction>> m_instructions;
};