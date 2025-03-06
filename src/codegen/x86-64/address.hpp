#pragma once

#include "../operand.hpp"
#include "codegen/x86-64/olabel.hpp"
#include "codegen/x86-64/register.hpp"
#include "type_info.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>

namespace CodeGen::x86_64::Operands {

struct Address : Operand {
    Address() : Operand(Type::Address, TypeInfo{}) {}
    Address(std::shared_ptr<Operand> base, const TypeInfo& ti) : Operand(Type::Address, ti), m_base(base) {}
    Address(std::shared_ptr<Operand> base, size_t offset, const TypeInfo& ti) : Operand(Type::Address, ti), m_base(base), m_offset(offset) {}

    std::shared_ptr<Operand> m_base;
    int64_t m_offset = 0;

    virtual void emit(std::ostream& os, const BrawContext& ctx) const override {
        switch(getSize(m_typeInfo)) {
            case Size::Byte: os << (ctx.m_assembler == GAS ? "BYTE PTR " : "byte "); break;
            case Size::Word: os << (ctx.m_assembler == GAS ? "WORD PTR " : "word "); break;
            case Size::Dword: os << (ctx.m_assembler == GAS ? "DWORD PTR " : "dword "); break;
            case Size::Qword: os << (ctx.m_assembler == GAS ? "QWORD PTR " : "qword "); break;
            case Size::Oword: os << (ctx.m_assembler == GAS ? "OWORD PTR " : "oword "); break;
            case Size::Yword: os << (ctx.m_assembler == GAS ? "YWORD PTR " : "yword "); break;
            default: break;
        }

        if(m_base->m_type == Operand::Type::Register) {
            auto reg = std::dynamic_pointer_cast<Operands::Register>(m_base);

            os << '[' << reg->m_ids.at(Size::Qword);
            if(m_offset != 0)
                os << (m_offset > 0 ? "+" : "") << m_offset;
            os << ']';
        }
        else {
            auto label = std::dynamic_pointer_cast<Operands::Label>(m_base);
            os << "[rip+" << label->m_id << ']';
        }
    }

    virtual std::shared_ptr<Operand> clone() const override {
        auto base = m_base->clone();
        auto ptr = std::make_shared<Address>(base, m_offset, m_typeInfo);
        return ptr;
    }
};

}