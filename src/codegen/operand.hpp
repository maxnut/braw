#pragma once

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

    enum class ValueType {
        Signed,
        Unsigned,
        SinglePrecision,
        DoublePrecision,
        Pointer,
        Function,
        Count
    };

    enum Size {
        Byte = 1,
        Word = 2,
        Dword = 4,
        Qword = 8,
        Oword = 16,
        Yword = 32
    };

    Operand(Type t, ValueType vt, Size size) : m_type(t), m_valueType(vt), m_size(size) {}
    virtual ~Operand() = default;

    virtual void emit(std::ostream& os) const = 0;
    virtual ValueType getValueType() const {return m_valueType;}
    virtual void setValueType(ValueType vt) {m_valueType = vt;}
    virtual Size getSize() const {return m_size;}
    virtual void setSize(Size size) {m_size = size;}

    virtual std::shared_ptr<Operand> clone() const = 0;

    friend std::ostream& operator<<(std::ostream& os, const Operand& obj) {
        obj.emit(os);
        return os;
    }

public:
    Type m_type;

protected:
    ValueType m_valueType = ValueType::Count;
    Size m_size = Dword;
};

}