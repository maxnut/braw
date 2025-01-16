#pragma once

#include "stack.hpp"
#include "memory.hpp"
#include "utils.hpp"

#include <vector>
#include <iostream>
#include <cstring>

namespace NativeFunctions {

template <typename T>
inline void print(Stack& stack, Memory* returnValue) {
    T val = Utils::fromStack<T, sizeof(T)>(stack);
    std::cout << val << "\n";
}

inline void scan(Stack& stack, Memory* returnValue) {
    int a = Utils::fromStack<int, sizeof(int)>(stack);
    char* b = Utils::fromStack<char*, sizeof(int) + sizeof(char*)>(stack);

    std::cin.getline(b, a);
}

inline void memory_allocate(Stack& stack, Memory* returnValue) {
    returnValue->from(malloc(Utils::fromStack<int, sizeof(int)>(stack)));
}

inline void memory_free(Stack& stack, Memory* returnValue) {
    free(Utils::fromStack<void*, sizeof(void*)>(stack));
}

inline void memory_set(Stack& stack, Memory* returnValue) {
    void* ptr = Utils::fromStack<void*, sizeof(void*) + sizeof(int) * 2>(stack);
    int value = Utils::fromStack<int, sizeof(int) * 2>(stack);
    int size = Utils::fromStack<int, sizeof(int)>(stack);
    std::memset(ptr, value, size);
}

}