#pragma once

#include "../operand.hpp"

#include <cstdint>

namespace CodeGen::x86_64 {

struct Integer : Operand {
    Integer() : Operand(Type::Integer) {}
    Integer(int64_t value) : Operand(Type::Integer), m_value(value) {}

    int64_t m_value;

    virtual void emit(std::ostream& os) const override {
        os << m_value;
    }
};

}