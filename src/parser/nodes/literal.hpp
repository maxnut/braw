#pragma once

#include "evaluatable.hpp"

#include <memory>

class LiteralNode : public EvaluatableNode {
public:
    std::unique_ptr<EvaluatableNode> m_literal;
};