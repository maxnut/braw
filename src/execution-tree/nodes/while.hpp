#pragma once

#include "evaluatable.hpp"
#include "scope.hpp"

#include <memory>

class WhileNode : public FunctionInstructionNode {
public:
    WhileNode() : FunctionInstructionNode(Type::While) {}

    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        interpreter.visitWhile(this, stack, functionContext);
    }

public:
    std::unique_ptr<EvaluatableNode> m_condition;
    std::unique_ptr<ScopeNode> m_scope;
};