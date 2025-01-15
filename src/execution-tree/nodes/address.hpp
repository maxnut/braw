#pragma once

#include "evaluatable.hpp"

#include <memory>

class AddressNode : public EvaluatableNode {
public:
    AddressNode() : EvaluatableNode(Type::Address) {}

    virtual Memory evaluate(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        return interpreter.visitAddress(this, stack, functionContext);
    }
    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        void* head = stack.head();
        interpreter.visitAddress(this, stack, functionContext);
        stack.setHead(head);
    }

public:
    std::unique_ptr<EvaluatableNode> m_base;
};