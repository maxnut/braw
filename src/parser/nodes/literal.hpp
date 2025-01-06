#pragma once

#include "evaluatable.hpp"

#include <memory>

class LiteralNode : public EvaluatableNode {
public:
    Memory m_value;
};