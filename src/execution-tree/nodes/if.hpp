#pragma once

#include "evaluatable.hpp"
#include "scope.hpp"

#include <memory>

class IfNode : public FunctionInstructionNode {
public:
    IfNode() : FunctionInstructionNode(Type::If) {}

    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        interpreter.visitIf(this, stack, functionContext);
    }

public:
    std::unique_ptr<EvaluatableNode> m_condition;
    std::unique_ptr<ScopeNode> m_scope;
    std::unique_ptr<FunctionInstructionNode> m_else = nullptr;
};