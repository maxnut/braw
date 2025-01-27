#pragma once

#include "../operand.hpp"

#include <cstddef>
#include <memory>

namespace CodeGen::x86_64::Operands {

struct Address : Operand {
    Address() : Operand(Type::Address, ValueType::Signed) {}
    Address(std::shared_ptr<Operand> base, ValueType vt = ValueType::Signed) : Operand(Type::Address, vt), m_base(base) {}
    Address(std::shared_ptr<Operand> base, size_t offset, ValueType vt = ValueType::Signed) : Operand(Type::Address, vt), m_base(base), m_offset(offset) {}

    std::shared_ptr<Operand> m_base;
    size_t m_offset = 0;

    virtual void emit(std::ostream& os) const override {
        os << '[' << *m_base;
        if(m_offset != 0)
            os << " + " << m_offset;
        os << ']';
    }
};

}