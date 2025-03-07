#pragma once

#include "../operand.hpp"
#include <cstdint>

namespace CodeGen::x86_64::Operands {

struct Immediate : Operand {
    Immediate() : Operand(Type::Label, TypeInfo{}) {}
    Immediate(int64_t value, const TypeInfo& ti) : Operand(Type::Immediate, ti), m_value(value) {}

    int64_t m_value;

    virtual std::shared_ptr<Operand> clone() const override {return std::make_shared<Immediate>(m_value, m_typeInfo);}
};

}