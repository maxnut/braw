#pragma once

#include "function_instruction.hpp"

#include <memory>
#include <vector>

class ScopeNode : public FunctionInstructionNode {
public:
    std::vector<std::unique_ptr<FunctionInstructionNode>> m_instructions;
};