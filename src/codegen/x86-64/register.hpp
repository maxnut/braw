#pragma once

#include "../operand.hpp"

namespace CodeGen::x86_64::Operands {

struct Register : Operand {
    Register() : Operand(Type::Register, ValueType::Signed) {}
    Register(const std::string& id) : Operand(Type::Register, ValueType::Signed), m_id(id) {}

    std::string m_id;

    virtual void emit(std::ostream& os) const override {
        os << m_id;
    }
};

}