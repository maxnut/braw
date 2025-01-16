#pragma once

#include "function_instruction.hpp"

#include <memory>
#include <vector>

class ScopeNode : public FunctionInstructionNode {
public:
    ScopeNode() : FunctionInstructionNode(Type::Scope) {}

    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) override {
        interpreter.executeScope(this, stack, functionContext);
    }

public:
    std::vector<std::unique_ptr<FunctionInstructionNode>> m_instructions;
};