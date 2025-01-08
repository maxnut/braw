#pragma once

#include "evaluatable.hpp"

#include <memory>

class ReturnNode : public FunctionInstructionNode {
public:
    std::unique_ptr<EvaluatableNode> m_value = nullptr;
};