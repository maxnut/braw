#pragma once

#include "ir/value.hpp"
#include "label.hpp"
#include "instruction.hpp"
#include "type_info.hpp"

#include <vector>

namespace CodeGen::x86_64 {

struct DataSection {
    std::vector<std::pair<std::string, Value>> m_labels;
    std::vector<std::pair<std::string, TypeInfo>> m_retains;
};

struct TextSection {
    std::vector<Instruction> m_instructions;
    std::vector<Label> m_globals;
    std::vector<Label> m_externals;
};

struct File {
    DataSection m_data;
    TextSection m_text;
};

}