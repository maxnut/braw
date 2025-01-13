#pragma once

#include "memory.hpp"
#include "stack.hpp"

#include <string>
#include <unordered_map>
#include <functional>

struct MemberInfo {
    std::string m_type;
    size_t m_offset = 0;
    size_t m_arraySize = 0;
};

struct OperatorInfo {
    std::function<void(Memory&, Memory&, Memory&)> m_function;
    std::string m_returnType;
};

struct CastInfo {
    std::function<Memory(Memory&, Stack&)> m_function;
};

struct TypeInfo {
    std::string m_name = "";
    size_t m_size = 0;
    std::unordered_map<std::string, OperatorInfo> m_operators;
    std::unordered_map<std::string, CastInfo> m_casts;
    std::unordered_map<std::string, MemberInfo> m_members;

    friend bool operator==(const TypeInfo&, const TypeInfo&);
};

inline bool operator==(const TypeInfo& lhs, const TypeInfo& rhs) {
    return lhs.m_name == rhs.m_name &&
        lhs.m_size == rhs.m_size &&
        lhs.m_members.size() == rhs.m_members.size();
}