#pragma once

#include "evaluatable.hpp"

#include <memory>

class ArrowNode : public EvaluatableNode {
public:
    ArrowNode() : EvaluatableNode(Type::Arrow) {}

    virtual Memory evaluate(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        return interpreter.visitArrow(this, stack, functionContext);
    }
    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        void* head = stack.head();
        interpreter.visitArrow(this, stack, functionContext);
        stack.setHead(head);
    }

public:
    std::unique_ptr<EvaluatableNode> m_base;
    size_t m_offset;
};