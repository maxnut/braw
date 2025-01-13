#pragma once

#include "evaluatable.hpp"

#include <memory>
#include <functional>

class CastNode : public EvaluatableNode {
public:
    virtual Memory evaluate(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        return interpreter.visitCast(this, stack, functionContext);
    }
    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        interpreter.visitCast(this, stack, functionContext);
    }

public:
    std::unique_ptr<EvaluatableNode> m_base;
    std::function<Memory(Memory&, Stack&)> m_function;
};