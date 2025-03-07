#pragma once

#include "../operand.hpp"
#include "braw_context.hpp"
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

    Register() : Operand(Type::Register, TypeInfo{}) {}
    Register(const std::unordered_map<Size, std::string>& ids, const TypeInfo& ti, RegisterType type, RegisterGroup group) : Operand(Type::Register, ti), m_ids(ids), m_registerType(type), m_group(group) {}

    virtual void emit(std::ostream& os, const BrawContext& ctx) const override {
        if(!m_ids.contains(getSize(m_typeInfo))) {
            os << "!err";
            return;
        }

        os << m_ids.at(getSize(m_typeInfo));
    }

    virtual std::shared_ptr<Operand> clone() const override {return std::make_shared<Register>(m_ids, m_typeInfo, m_registerType, m_group);}

    RegisterType m_registerType = General;
    RegisterGroup m_group = Count;
    std::unordered_map<Size, std::string> m_ids;
};

}