#pragma once

#include "function_definition.hpp"
#include "evaluatable.hpp"

#include <memory>
#include <vector>

class FunctionCallNode : public EvaluatableNode {
public:
    virtual Memory evaluate(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        return interpreter.visitFunctionCall(this, stack, functionContext);
    }
    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        void* head = stack.head();
        interpreter.visitFunctionCall(this, stack, functionContext);
        stack.setHead(head);
    }

public:
    std::shared_ptr<FunctionDefinitionNode> m_function;
    std::vector<std::unique_ptr<EvaluatableNode>> m_parameters;
};