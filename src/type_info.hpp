#pragma once

#include <string>
#include <unordered_map>

struct MemberInfo {
    std::string m_type;
    size_t m_offset = 0;
    size_t m_arraySize = 0;
};

struct TypeInfo {
    std::string m_name = "";
    size_t m_size = 0;
    std::unordered_map<std::string, MemberInfo> m_members;

    bool operator==(const TypeInfo& t2) {
        return m_name == t2.m_name &&
        m_size == t2.m_size &&
        m_members.size() == t2.m_members.size();
    }
};