#pragma once

#include "function_definition.hpp"
#include "evaluatable.hpp"

#include <memory>
#include <vector>

class NativeFunctionCallNode : public EvaluatableNode {
public:
    virtual Memory evaluate(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        return interpreter.visitNativeFunctionCall(this, stack, functionContext);
    }
    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        interpreter.visitNativeFunctionCall(this, stack, functionContext);
    }

public:
    std::shared_ptr<FunctionDefinitionNode> m_function;
    std::vector<std::unique_ptr<EvaluatableNode>> m_parameters;
};