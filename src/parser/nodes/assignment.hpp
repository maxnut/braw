#pragma once

#include "evaluatable.hpp"

#include <memory>

class AssignmentNode : public FunctionInstructionNode {
public:
    std::unique_ptr<EvaluatableNode> m_variable;
    std::unique_ptr<EvaluatableNode> m_value;
};