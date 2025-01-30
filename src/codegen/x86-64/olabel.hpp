#pragma once

#include "../operand.hpp"

namespace CodeGen::x86_64::Operands {

struct Label : Operand {
    Label() : Operand(Type::Label, ValueType::Pointer, Size::Qword) {}
    Label(const std::string& id, ValueType vtype, Size size) : Operand(Type::Label, vtype, size), m_id(id) {}

    std::string m_id;

    virtual void emit(std::ostream& os) const override {
        os << m_id;
    }
    virtual std::shared_ptr<Operand> clone() const override {return std::make_shared<Label>(m_id, m_valueType, m_size);}
};

}