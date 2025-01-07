#pragma once

#include "evaluatable.hpp"
#include "scope.hpp"

#include <memory>

class IfNode : public FunctionInstructionNode {
public:
    std::unique_ptr<EvaluatableNode> m_condition;
    std::unique_ptr<ScopeNode> m_scope;
};