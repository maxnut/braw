#pragma once

#include "memory.hpp"
#include "type_info.hpp"
#include "function_instruction.hpp"

enum ValueType {
    DECLARATION,
    LVALUE,
    PRVALUE,
    XVALUE,
    COUNT
};

static std::string valueTypeString(ValueType type) {
    switch(type) {
        case DECLARATION: return "DECLARATION";
        case LVALUE: return "LVALUE";
        case PRVALUE: return "PRVALUE";
        case XVALUE: return "XVALUE";
        default: case COUNT: return "COUNT";
    }
}

class EvaluatableNode : public FunctionInstructionNode {
public:
    EvaluatableNode(Node::Type type) : FunctionInstructionNode(type) {}

    virtual Memory evaluate(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) = 0;

public:
    TypeInfo m_type;
    size_t m_size = 0;
    ValueType m_memoryType = COUNT;
};