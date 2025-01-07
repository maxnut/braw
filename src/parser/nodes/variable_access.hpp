#pragma once

#include "evaluatable.hpp"

#include <memory>

class VariableAccessNode : public EvaluatableNode {
public:
    size_t m_stackIndex = 0;
};