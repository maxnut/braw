#pragma once

#include "evaluatable.hpp"
#include "scope.hpp"

#include <memory>

class IfNode : public FunctionInstructionNode {
public:
    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        interpreter.visitIf(this, stack, functionContext);
    }

public:
    std::unique_ptr<EvaluatableNode> m_condition;
    std::unique_ptr<ScopeNode> m_scope;
};