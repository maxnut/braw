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

    virtual std::shared_ptr<Operand> clone() const = 0;

    static Size getSize(const TypeInfo& ti) {
        if(ti.m_name == INT_T) {
            return Size::Dword;
        }
        else if(ti.m_name == LONG_T) {
            return Size::Qword;
        }
        else if(ti.m_name == BOOL_T) {
            return Size::Byte;
        }
        else if(ti.m_name == CHAR_T) {
            return Size::Byte;
        }
        else if(ti.m_name == FLOAT_T) {
            return Size::Dword;
        }
        else if(ti.m_name == DOUBLE_T) {
            return Size::Qword;
        }
        return Size::Qword;
    }

public:
    Type m_type;
    TypeInfo m_typeInfo{};
    size_t m_scale = 1;
};

}