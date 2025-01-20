#pragma once

#include "instruction.hpp"

#include <string>
#include <vector>
#include <memory>

struct Label {
    std::string m_id;
    std::vector<std::unique_ptr<Instruction>> m_instructions;
};