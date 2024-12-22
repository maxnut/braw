#pragma once

#include "evaluatable.hpp"

#include <memory>

class VariableDeclarationNode : public FunctionInstructionNode {
public:
    TypeInfo m_type;
    std::unique_ptr<EvaluatableNode> m_assignmentValue = nullptr;
};