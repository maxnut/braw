#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

constexpr const char* VOID_T = "void";
constexpr const char* INT_T = "int";
constexpr const char* LONG_T = "long";
constexpr const char* FLOAT_T = "float";
constexpr const char* DOUBLE_T = "double";
constexpr const char* BOOL_T = "bool";
constexpr const char* CHAR_T = "char";

struct MemberInfo {
    std::string m_type;
    size_t m_offset = 0;
    size_t m_scale = 0;
};

struct OperatorInfo {
    std::string m_returnType;
};

struct TypeInfo {
    std::string m_name = "";
    size_t m_size = 0;
    bool m_builtin = false;
    std::unordered_map<std::string, OperatorInfo> m_operators;
    std::unordered_set<std::string> m_validCasts;
    std::unordered_map<std::string, MemberInfo> m_members;

    std::optional<MemberInfo> memberByOffset(size_t offset) {
        for(auto& pair : m_members) {
            if(pair.second.m_offset == offset)
                return pair.second;
        }
        return std::nullopt;
    }

    friend bool operator==(const TypeInfo&, const TypeInfo&);
};

inline bool operator==(const TypeInfo& lhs, const TypeInfo& rhs) {
    return lhs.m_name == rhs.m_name &&
        lhs.m_size == rhs.m_size &&
        lhs.m_members.size() == rhs.m_members.size();
}