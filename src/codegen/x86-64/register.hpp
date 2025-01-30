#pragma once

#include "../operand.hpp"

namespace CodeGen::x86_64::Operands {

struct Register : Operand {
    enum RegisterType {
        General,
        Simd
    };

    Register() : Operand(Type::Register, ValueType::Count, Size::Byte) {}
    Register(const std::string& id, Size size, ValueType vtype, RegisterType registerType) : Operand(Type::Register, vtype, size), m_id(id), m_registerType(registerType) {}

    virtual void emit(std::ostream& os) const override {
        os << m_id;
    }

    virtual std::shared_ptr<Operand> clone() const override {return std::make_shared<Register>(m_id, m_size, m_valueType, m_registerType);}

    std::string m_id;
    RegisterType m_registerType = General;
};

}