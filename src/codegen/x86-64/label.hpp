#pragma once

#include "instruction.hpp"

namespace CodeGen::x86_64 {

struct Label {
    std::string m_id;
    std::vector<Instruction> m_instructions;
};

}