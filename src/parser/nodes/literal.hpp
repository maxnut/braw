#pragma once

#include "evaluatable.hpp"

#include <memory>

class LiteralNode : public EvaluatableNode {
public:
    virtual Memory evaluate(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        return interpreter.visitLiteral(this, stack, functionContext);
    }
    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        interpreter.visitLiteral(this, stack, functionContext);
    }

public:
    Memory m_value;
};