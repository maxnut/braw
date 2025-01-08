#pragma once

#include "evaluatable.hpp"

#include <memory>

class DereferenceNode : public EvaluatableNode {
public:
    virtual Memory evaluate(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        return interpreter.visitDereference(this, stack, functionContext);
    }
    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        interpreter.visitDereference(this, stack, functionContext);
    }

public:
    std::unique_ptr<EvaluatableNode> m_base;
};