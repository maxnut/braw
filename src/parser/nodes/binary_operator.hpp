#pragma once

#include "evaluatable.hpp"

#include <memory>
#include <functional>

class BinaryOperatorNode : public EvaluatableNode {
public:
    virtual Memory evaluate(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        return interpreter.visitBinaryOperator(this, stack, functionContext);
    }
    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        void* head = stack.head();
        interpreter.visitBinaryOperator(this, stack, functionContext);
        stack.setHead(head);
    }

public:
    std::unique_ptr<EvaluatableNode> m_left;
    std::unique_ptr<EvaluatableNode> m_right;
    std::function<void(Memory&, Memory&, Memory&)> m_function;
};