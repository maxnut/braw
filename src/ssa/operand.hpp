#pragma once

#include "type_info.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <variant>

namespace SSA {

struct Operand {
    enum Type {
        Register,
        Immediate,
        Address
    };

    Operand(Type t) : m_type(t) {}
    Operand(Type t, const TypeInfo& ti) : m_type(t), m_typeInfo(ti) {}

    Type m_type;
    TypeInfo m_typeInfo;
    size_t m_scale = 1;
};

struct Register : Operand {
    Register() : Operand(Type::Register) {}
    Register(const std::string& id) : Operand(Type::Register), m_id(id), m_originalId(id) {}
    Register(const std::string& id, const TypeInfo& t) : Operand(Type::Register, t), m_id(id), m_originalId(id) {}

    std::string m_id;
    std::string m_originalId;
    bool m_memoryDependant = false;
    bool m_isPhi = false;
};

struct Immediate : Operand {
    Immediate() : Operand(Type::Immediate) {}
    Immediate(std::variant<int, long, float, double, bool, std::string, char, std::nullptr_t> value, const TypeInfo& ti) : Operand(Type::Immediate, ti), m_value(value) {}

    std::variant<int, long, float, double, bool, std::string, char, std::nullptr_t> m_value;
};

struct Address : Operand {
    Address() : Operand(Type::Address) {}
    Address(const TypeInfo& ti, std::shared_ptr<SSA::Register> base, int64_t offset = 0, std::shared_ptr<SSA::Register> index = nullptr, int64_t scaleSize = 0, int64_t scale = 0) : Operand(Type::Address, ti), m_base(base), m_offset(offset), m_index(index), m_scaleSize(scaleSize) {}

    std::shared_ptr<SSA::Register> m_base;
    int64_t m_offset = 0;
    std::shared_ptr<SSA::Register> m_index = nullptr;
    int64_t m_scaleSize = 0;
};

}