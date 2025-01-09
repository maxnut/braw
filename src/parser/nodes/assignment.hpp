#pragma once

#include "evaluatable.hpp"

#include <memory>

class AssignmentNode : public FunctionInstructionNode {
public:
    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        interpreter.visitAssignment(this, stack, functionContext);
    }

public:
    std::unique_ptr<EvaluatableNode> m_variable;
    std::unique_ptr<EvaluatableNode> m_value;
};