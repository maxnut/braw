#pragma once

#include "evaluatable.hpp"

#include <memory>
#include <vector>

class LiteralNode : public EvaluatableNode {
public:
    ~LiteralNode() override { free(m_value.m_data); }

    virtual Memory evaluate(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        return interpreter.visitLiteral(this, stack, functionContext);
    }
    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        void* head = stack.head();
        interpreter.visitLiteral(this, stack, functionContext);
        stack.setHead(head);
    }

public:
    Memory m_value;

public:
    static std::vector<std::shared_ptr<char>> s_strings;
};