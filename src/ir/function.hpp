#pragma once

#include "register.hpp"
#include "instruction.hpp"

#include <vector>
#include <memory>

struct Function {
    std::string m_name;
    std::vector<std::shared_ptr<Register>> m_args;
    std::shared_ptr<Register> m_optReturn = nullptr;
    std::vector<std::unique_ptr<Instruction>> m_instructions;
    bool m_external = false;
    std::unordered_map<std::string, std::shared_ptr<Register>> m_retains;

    Function() = default;
    Function(Function&&) noexcept = default;
    Function& operator=(Function&&) noexcept = default;

    Function(const Function&) = delete;
    Function& operator=(const Function&) = delete;
};