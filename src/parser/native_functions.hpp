#pragma once

#include "../stack.hpp"
#include "nodes/function_definition.hpp"

#include <vector>
#include <iostream>
#include <cstring>

namespace NativeFunctions {

inline void print(Stack& stack, Memory* returnValue, const std::vector<TypeInfo>& parameters) {
    TypeInfo param = parameters[0];
    Memory message{stack.ptrHead(param.m_size), param.m_size};

    if(param.m_name == "int") {
        int a = message.get<int>();

        std::cout << a << "\n";
    }
    else if(param.m_name == "float") {
        float a = message.get<float>();

        std::cout << a << "\n";
    }
    else if(param.m_name == "bool") {
        bool a = message.get<bool>();

        std::cout << (a ? "true" : "false") << "\n";
    }
    else if(param.m_name == "char") {
        char a = message.get<char>();

        std::cout << a << "\n";
    }
    else if(param.m_name == "char*") {
        char* a = message.get<char*>();

        std::cout << a << "\n";
    }
}

inline void scan(Stack& stack, Memory* returnValue, const std::vector<TypeInfo>& parameters) {
    TypeInfo size = parameters[0];
    TypeInfo ptr = parameters[1];

    Memory sizeM{stack.ptrHead(ptr.m_size + size.m_size), size.m_size};
    Memory ptrM{stack.ptrHead(ptr.m_size), ptr.m_size};

    int a = sizeM.get<int>();
    char* b = ptrM.get<char*>();

    std::cin.getline(b, a);
}

inline void memory_allocate(Stack& stack, Memory* returnValue, const std::vector<TypeInfo>& parameters) {
    TypeInfo param = parameters[0];
    Memory message{stack.ptrHead(param.m_size), param.m_size};
    Memory ret = stack.push(8);
    ret.from(malloc(message.get<int>()));
    *returnValue = ret;
}

inline void memory_free(Stack& stack, Memory* returnValue, const std::vector<TypeInfo>& parameters) {
    TypeInfo param = parameters[0];
    Memory message{stack.ptrHead(param.m_size), param.m_size};
    free(message.get<void*>());
}

inline void memory_set(Stack& stack, Memory* returnValue, const std::vector<TypeInfo>& parameters) {
    Memory ptrM{stack.ptrHead(16), 8};
    Memory valueM{stack.ptrHead(8), 4};
    Memory sizeM{stack.ptrHead(4), 4};

    void* ptr = ptrM.get<void*>();
    int value = valueM.get<int>();
    int size = sizeM.get<int>();
    std::memset(ptr, value, size);
}

}