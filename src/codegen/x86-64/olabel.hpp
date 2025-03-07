#pragma once

#include "../operand.hpp"

namespace CodeGen::x86_64::Operands {

struct Label : Operand {
    Label() : Operand(Type::Label, TypeInfo{}) {}
    Label(const std::string& id, const TypeInfo& ti = TypeInfo{}) : Operand(Type::Label, ti), m_id(id) {}

    std::string m_id;

    virtual std::shared_ptr<Operand> clone() const override {return std::make_shared<Label>(m_id, m_typeInfo);}
};

}