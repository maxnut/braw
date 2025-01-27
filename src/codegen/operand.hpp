#pragma once

#include <ostream>

namespace CodeGen {

struct Operand {
    enum class Type {
        Register,
        Address,
        Label,
        Immediate,
        Count
    };

    enum class ValueType {
        Signed,
        Unsigned,
        SinglePrecision,
        DoublePrecision,
        Pointer,
        Function,
        Structure,
        Count
    };

    Operand(Type t, ValueType vt) : m_type(t), m_valueType(vt) {}
    virtual ~Operand() = default;

    Type m_type;
    ValueType m_valueType = ValueType::Count;


    virtual void emit(std::ostream& os) const = 0;

    friend std::ostream& operator<<(std::ostream& os, const Operand& obj) {
        obj.emit(os);
        return os;
    }
};

}