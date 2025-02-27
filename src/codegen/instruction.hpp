#pragma once

#include "operand.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace CodeGen {

struct Instruction {
    std::vector<std::shared_ptr<Operand>> m_operands;
    size_t m_irIndex = 0;
    size_t m_irFunctionIndex = 0;

    virtual void addOperand(std::shared_ptr<Operand> op) {
        assert(/* op->m_type != Operand::Type::Register ||  */op->getSize() != Operand::Size::Uninitialized);
        m_operands.push_back(op->clone());
    }

    virtual void opcodeInstruction(std::ostream& os) const = 0;

    friend std::ostream& operator<<(std::ostream& os, const Instruction& obj) {
        obj.opcodeInstruction(os);
        os << " ";
        for(int i = 0; i < obj.m_operands.size(); i++) {
            os << *obj.m_operands[i];
            if(i < obj.m_operands.size() - 1)
                os << ", ";
        }

        // os << "\t; ";

        // for(int i = 0; i < obj.m_operands.size(); i++) {
        //     auto op = obj.m_operands[i];
        //     os << "ValueType: ";
        //     switch(op->getValueType()) {
        //         case Operand::ValueType::Pointer: os << "Pointer"; break;
        //         case Operand::ValueType::Signed: os << "Signed"; break;
        //         case Operand::ValueType::Unsigned: os << "Unsigned"; break;
        //         case Operand::ValueType::SinglePrecision: os << "Single"; break;
        //         case Operand::ValueType::DoublePrecision: os << "Double"; break;
        //         case Operand::ValueType::Function: os << "Func"; break;
        //         case Operand::ValueType::Count: os << "Unknown"; break;
        //     }
        //     os << ", Size: " << op->getSize();
        //     if(i < obj.m_operands.size() - 1)
        //         os << " | ";
        // }

        return os;
    }
};

}