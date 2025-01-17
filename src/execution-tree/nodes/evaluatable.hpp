#pragma once

#include "memory.hpp"
#include "type_info.hpp"
#include "function_instruction.hpp"

class EvaluatableNode : public FunctionInstructionNode {
public:
    EvaluatableNode(Node::Type type) : FunctionInstructionNode(type) {}

    virtual Memory evaluate(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) = 0;

public:
    TypeInfo m_type;
    size_t m_size = 0;
};