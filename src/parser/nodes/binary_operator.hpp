#pragma once

#include "evaluatable.hpp"

#include <memory>

class BinaryOperatorNode : public EvaluatableNode {
public:
    std::unique_ptr<EvaluatableNode> m_left;
    std::unique_ptr<EvaluatableNode> m_right;
};