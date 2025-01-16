#pragma once

#include "evaluatable.hpp"

#include <memory>
#include <vector>
#include <variant>

class LiteralNode : public EvaluatableNode {
public:
    LiteralNode() : EvaluatableNode(Type::Literal) {}

    virtual Memory evaluate(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        return interpreter.visitLiteral(this, stack, functionContext);
    }
    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        void* head = stack.head();
        interpreter.visitLiteral(this, stack, functionContext);
        stack.setHead(head);
    }

public:
    std::variant<int, long, float, double, bool, std::string, std::nullptr_t> m_value;
};