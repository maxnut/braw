#pragma once

#include "evaluatable.hpp"

#include <memory>

class ReturnNode : public FunctionInstructionNode {
public:
    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        interpreter.visitReturn(this, stack, functionContext);
    }

public:
    std::unique_ptr<EvaluatableNode> m_value = nullptr;
};