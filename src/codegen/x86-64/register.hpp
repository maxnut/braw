#pragma once

#include "../operand.hpp"
#include <string>
#include <unordered_map>

namespace CodeGen::x86_64::Operands {

struct Register : Operand {
    enum RegisterGroup {
        RAX, RBX, RCX, RDX, 
        RSP, RBP, RSI, RDI,
        R8, R9, R10, R11, R12, R13, R14, R15,
        XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7,
        XMM8, XMM9, XMM10, XMM11, XMM12, XMM13, XMM14, XMM15,
        Count
    };

    enum RegisterType {
        General,
        Simd
    };

    Register() : Operand(Type::Register, ValueType::Count, Size::Byte) {}
    Register(const std::unordered_map<Size, std::string>& ids, Size size, ValueType vtype, RegisterType registerType, RegisterGroup group) : Operand(Type::Register, vtype, size), m_ids(ids), m_registerType(registerType), m_group(group) {}

    virtual void emit(std::ostream& os) const override {
        if(!m_ids.contains(m_size)) {
            os << "!err";
            return;
        }

        os << m_ids.at(m_size);
    }

    virtual std::shared_ptr<Operand> clone() const override {return std::make_shared<Register>(m_ids, m_size, m_valueType, m_registerType, m_group);}

    RegisterType m_registerType = General;
    RegisterGroup m_group = Count;
    std::unordered_map<Size, std::string> m_ids;
};

}