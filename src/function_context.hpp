#pragma once

#include "memory.hpp"

struct FunctionContext {
    Memory m_returnValue;
    size_t m_functionPtr = 0;
    bool m_return = false;
};