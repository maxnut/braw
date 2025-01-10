#pragma once

#include "evaluatable.hpp"

#include <memory>

class ArrowNode : public EvaluatableNode {
public:
    virtual Memory evaluate(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        return interpreter.visitArrow(this, stack, functionContext);
    }
    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        interpreter.visitArrow(this, stack, functionContext);
    }

public:
    std::unique_ptr<EvaluatableNode> m_base;
    size_t m_offset;
};