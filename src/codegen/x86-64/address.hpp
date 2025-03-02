#pragma once

#include "../operand.hpp"
#include "codegen/x86-64/olabel.hpp"
#include "codegen/x86-64/register.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>

namespace CodeGen::x86_64::Operands {

struct Address : Operand {
    Address() : Operand(Type::Address, ValueType::Pointer, Size::Qword) {}
    Address(std::shared_ptr<Operand> base, ValueType vt = ValueType::Pointer) : Operand(Type::Address, vt, Size::Qword), m_base(base) {}
    Address(std::shared_ptr<Operand> base, size_t offset, ValueType vt = ValueType::Pointer) : Operand(Type::Address, vt, Size::Qword), m_base(base), m_offset(offset) {}

    std::shared_ptr<Operand> m_base;
    int64_t m_offset = 0;

    virtual void emit(std::ostream& os) const override {
        if(m_base->m_type == Operand::Type::Register) {
            switch(m_size) {
                case Size::Byte: os << "byte "; break;
                case Size::Word: os << "word "; break;
                case Size::Dword: os << "dword "; break;
                case Size::Qword: os << "qword "; break;
                case Size::Oword: os << "oword "; break;
                case Size::Yword: os << "yword "; break;
                default: break;
            }
            
            auto reg = std::dynamic_pointer_cast<Operands::Register>(m_base);

            os << '[' << reg->m_ids.at(Size::Qword);
            if(m_offset != 0)
                os << (m_offset > 0 ? "+" : "") << m_offset;
            os << ']';
        }
        else {
            auto label = std::dynamic_pointer_cast<Operands::Label>(m_base);
            os << '[' << label->m_id << ']';
        }
    }

    virtual ValueType getValueType() const override {return m_valueType;}
    virtual void setValueType(ValueType vt) override {m_valueType = vt; m_base->setValueType(ValueType::Pointer);}
    virtual Size getSize() const override {return m_size;}
    virtual void setSize(Size size) override {m_size = size; m_base->setSize(Size::Qword);}

    virtual std::shared_ptr<Operand> clone() const override {
        auto base = m_base->clone();
        auto ptr = std::make_shared<Address>(base, m_offset, getValueType());
        ptr->setSize(getSize());
        return ptr;
    }
};

}