#pragma once

#include "../operand.hpp"

#include <cstddef>
#include <memory>

namespace CodeGen::x86_64::Operands {

struct Address : Operand {
    Address() : Operand(Type::Address, ValueType::Count) {}
    Address(std::shared_ptr<Operand> base, ValueType vt = ValueType::Count) : Operand(Type::Address, vt), m_base(base) {}
    Address(std::shared_ptr<Operand> base, size_t offset, ValueType vt = ValueType::Count) : Operand(Type::Address, vt), m_base(base), m_offset(offset) {}

    std::shared_ptr<Operand> m_base;
    size_t m_offset = 0;

    virtual void emit(std::ostream& os) const override {
        os << '[' << *m_base;
        if(m_offset != 0)
            os << " + " << m_offset;
        os << ']';
    }

    virtual ValueType getValueType() const override {return m_base->getValueType();}
    virtual void setValueType(ValueType vt) override {m_base->setValueType(vt);}
};

}