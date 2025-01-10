#pragma once

#include "stack.hpp"
#include "memory.hpp"
#include "type_info.hpp"

namespace Utils {
    template <typename T>
    T parameterFromStack(Stack& stack, uint8_t parameterIndex, const std::vector<TypeInfo>& parameters) {
        size_t off = 0;

        for(uint32_t i = parameters.size() - 1; i > parameterIndex; i--)
            off += parameters[i].m_size;
        Memory message{stack.ptrHead(off), parameters[parameterIndex].m_size};

        return message.get<T>();
    }
}