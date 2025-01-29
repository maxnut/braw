#pragma once

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

    virtual void emit(std::ostream& os) const = 0;
    virtual ValueType getValueType() const {return m_valueType;}
    virtual void setValueType(ValueType vt) {m_valueType = vt;}

    friend std::ostream& operator<<(std::ostream& os, const Operand& obj) {
        obj.emit(os);
        return os;
    }

public:
    Type m_type;

protected:
    ValueType m_valueType = ValueType::Count;
};

}