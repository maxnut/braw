#pragma once

#include "ssa/instruction.hpp"
#include "ssa/operand.hpp"
#include <filesystem>
#include <vector>

namespace SSA {

struct Function {
    std::string m_name;
    std::vector<std::shared_ptr<Register>> m_args;
    std::shared_ptr<Register> m_optReturn = nullptr;
    std::vector<std::shared_ptr<Instruction>> m_instructions;
    bool m_external = false;
    std::unordered_map<std::string, std::shared_ptr<Register>> m_retains;
};

struct File {
    std::vector<Function> m_functions;
    std::filesystem::path m_path;
};

}