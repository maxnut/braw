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
    Address(std::shared_ptr<Operand> base, size_t offset, const TypeInfo& ti, std::shared_ptr<Operands::Register> index, size_t scale) : Operand(Type::Address, ti), m_base(base), m_offset(offset), m_index(index), m_scale(scale) {}

    std::shared_ptr<Operand> m_base;
    int64_t m_offset = 0;
    std::shared_ptr<Operands::Register> m_index = nullptr;
    int64_t m_scale = 0;

    virtual std::shared_ptr<Operand> clone() const override {
        auto base = m_base->clone();
        auto ptr = std::make_shared<Address>(base, m_offset, m_typeInfo, m_index, m_scale);
        return ptr;
    }
};

}