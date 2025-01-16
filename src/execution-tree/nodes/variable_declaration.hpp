#pragma once

#include "evaluatable.hpp"

#include <memory>

class VariableDeclarationNode : public FunctionInstructionNode {
public:
    VariableDeclarationNode() : FunctionInstructionNode(Type::VariableDeclaration) {}

    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        interpreter.visitVariableDeclaration(this, stack, functionContext);
    }

public:
    TypeInfo m_type;
    size_t size;
    std::unique_ptr<EvaluatableNode> m_assignmentValue = nullptr;
};