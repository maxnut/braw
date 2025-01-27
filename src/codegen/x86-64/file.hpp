#pragma once

#include "ir/value.hpp"
#include "label.hpp"
#include "instruction.hpp"

#include <unordered_map>
#include <vector>

namespace CodeGen::x86_64 {

struct DataSection {
    std::vector<std::pair<std::string, Value>> m_labels;
};

struct TextSection {
    std::vector<Instruction> m_instructions;
    std::unordered_map<uint32_t, Label> m_labels;
};

struct File {
    DataSection m_data;
    TextSection m_text;
};

}