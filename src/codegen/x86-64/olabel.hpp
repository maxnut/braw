#pragma once

#include "../operand.hpp"

namespace CodeGen::x86_64::Operands {

struct Label : Operand {
    Label() : Operand(Type::Label) {}
    Label(const std::string& id) : Operand(Type::Label), m_id(id) {}

    std::string m_id;

    virtual void emit(std::ostream& os) const override {
        os << m_id;
    }
};

}