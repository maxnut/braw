#pragma once

#include "memory.hpp"
#include "type_info.hpp"
#include "function_instruction.hpp"

enum ValueType {
    LVALUE,
    PRVALUE,
    XVALUE
};

class EvaluatableNode : public FunctionInstructionNode {
public:
    TypeInfo m_type;
    size_t m_size;
    ValueType m_memoryType;
};