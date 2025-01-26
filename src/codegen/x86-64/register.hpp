#pragma once

#include "../operand.hpp"

namespace CodeGen::x86_64 {

struct Register : Operand {
    Register() : Operand(Type::Register) {}
    Register(const std::string& id) : Operand(Type::Register), m_id(id) {}

    std::string m_id;

    virtual void emit(std::ostream& os) const override {
        os << m_id;
    }
};

}