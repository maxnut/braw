#pragma once

#include "braw_context.hpp"
#include <memory>
#include <ostream>

namespace CodeGen {

class Operand {
public:
    enum class Type {
        Register,
        Address,
        Label,
        Immediate,
        Count
    };

    enum Size {
        Byte = 1,
        Word = 2,
        Dword = 4,
        Qword = 8,
        Oword = 16,
        Yword = 32,
        Uninitialized = -1
    };

    Operand(Type t, const TypeInfo& ti) : m_type(t), m_typeInfo(ti) {}
    virtual ~Operand() = default;

    virtual void emit(std::ostream& os, const BrawContext& ctx) const = 0;

    virtual std::shared_ptr<Operand> clone() const = 0;

    static Size getSize(const TypeInfo& ti) {
        if(ti.m_name == "int") {
            return Size::Dword;
        }
        else if(ti.m_name == "long") {
            return Size::Qword;
        }
        else if(ti.m_name == "bool") {
            return Size::Byte;
        }
        else if(ti.m_name == "char") {
            return Size::Byte;
        }
        else if(ti.m_name == "float") {
            return Size::Dword;
        }
        else if(ti.m_name == "double") {
            return Size::Qword;
        }
        return Size::Qword;
    }

public:
    Type m_type;
    TypeInfo m_typeInfo{};
};

}