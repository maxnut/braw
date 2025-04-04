#pragma once

#include "instruction.hpp"

#include <string>
#include <utility>

struct Label : Instruction {
    Label(std::pair<uint32_t, uint32_t> pos) : Instruction(Type::Label, pos) {}

    std::string m_id;
};