#pragma once

#include "../operand.hpp"

#include <cstddef>
#include <memory>

namespace CodeGen::x86_64::Operands {

struct Address : Operand {
    Address() : Operand(Type::Address, ValueType::Pointer, Size::Qword) {}
    Address(std::shared_ptr<Operand> base, ValueType vt = ValueType::Pointer) : Operand(Type::Address, vt, Size::Qword), m_base(base) {}
    Address(std::shared_ptr<Operand> base, size_t offset, ValueType vt = ValueType::Pointer) : Operand(Type::Address, vt, Size::Qword), m_base(base), m_offset(offset) {}

    std::shared_ptr<Operand> m_base;
    size_t m_offset = 0;

    virtual void emit(std::ostream& os) const override {
        switch(m_base->getSize()) {
            case Size::Byte: os << "byte "; break;
            case Size::Word: os << "word "; break;
            case Size::Dword: os << "dword "; break;
            case Size::Qword: os << "qword "; break;
            case Size::Oword: os << "oword "; break;
            case Size::Yword: os << "yword "; break;
            default: break;
        }

        os << '[' << *m_base;
        if(m_offset != 0)
            os << "+" << m_offset;
        os << ']';
    }

    virtual ValueType getValueType() const override {return m_base->getValueType();}
    virtual void setValueType(ValueType vt) override {m_base->setValueType(vt);}
    virtual Size getSize() const override {return m_base->getSize();}
    virtual void setSize(Size size) override {m_base->setSize(size);}

    virtual std::shared_ptr<Operand> clone() const override {
        auto base = m_base->clone();
        return std::make_shared<Address>(base, m_offset, getValueType());
    }
};

}