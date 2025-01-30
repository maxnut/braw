#pragma once

#include "../operand.hpp"
#include <cstdint>

namespace CodeGen::x86_64::Operands {

struct Immediate : Operand {
    Immediate() : Operand(Type::Label, ValueType::Signed, Size::Qword) {}
    Immediate(int64_t value, ValueType valueType, Size size) : Operand(Type::Immediate, valueType, size), m_value(value) {}

    int64_t m_value;

    virtual void emit(std::ostream& os) const override {
        os << m_value;
    }
    virtual std::shared_ptr<Operand> clone() const override {return std::make_shared<Immediate>(m_value, m_valueType, m_size);}
};

}