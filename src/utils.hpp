#pragma once

#include "stack.hpp"

namespace Utils {
    template <typename T, size_t offset>
    T fromStack(Stack& stack) {
        return *(T*)(stack.ptrHead(offset));
    }
}