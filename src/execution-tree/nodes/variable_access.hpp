#pragma once

#include "evaluatable.hpp"

#include <memory>

class VariableAccessNode : public EvaluatableNode {
public:
    virtual Memory evaluate(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        return interpreter.visitVariableAccess(this, stack, functionContext);
    }
    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        void* head = stack.head();
        interpreter.visitVariableAccess(this, stack, functionContext);
        stack.setHead(head);
    }

public:
    size_t m_stackIndex = 0;
};