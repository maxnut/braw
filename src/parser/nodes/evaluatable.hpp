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

class EvaluatableNode : public FunctionInstructionNode {
public:
    TypeInfo m_type;
    size_t m_size = 0;
    ValueType m_memoryType = COUNT;
};