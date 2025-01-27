#pragma once

#include "../operand.hpp"
#include <cstdint>

namespace CodeGen::x86_64::Operands {

struct Immediate : Operand {
    Immediate() : Operand(Type::Label, ValueType::Signed) {}
    Immediate(int64_t value) : Operand(Type::Immediate, ValueType::Signed), m_value(value) {}

    int64_t m_value;

    virtual void emit(std::ostream& os) const override {
        os << m_value;
    }
};

}