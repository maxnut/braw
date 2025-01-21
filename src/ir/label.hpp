#pragma once

#include "instruction.hpp"

#include <string>

struct Label : Instruction {
    Label() : Instruction(Type::Label) {}

    std::string m_id;
};