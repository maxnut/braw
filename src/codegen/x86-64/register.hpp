#pragma once

#include "../operand.hpp"

namespace CodeGen::x86_64::Operands {

struct Register : Operand {
    enum Size {
        Byte = 1,
        Word = 2,
        Dword = 4,
        Qword = 8,
        Oword = 16,
        Yword = 32
    };

    enum RegisterType {
        General,
        Simd
    };

    Register() : Operand(Type::Register, ValueType::Count) {}
    Register(const std::string& id, Size size, ValueType vtype, RegisterType registerType) : Operand(Type::Register, vtype), m_id(id), m_size(size), m_registerType(registerType) {}

    virtual void emit(std::ostream& os) const override {
        os << m_id;
    }

    std::string m_id;
    Size m_size = Dword;
    RegisterType m_registerType = General;
};

}