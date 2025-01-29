#pragma once

#include "operand.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace CodeGen {

struct Instruction {
    std::vector<std::shared_ptr<Operand>> m_operands;

    virtual void opcodeInstruction(std::ostream& os) const = 0;

    friend std::ostream& operator<<(std::ostream& os, const Instruction& obj) {
        obj.opcodeInstruction(os);
        os << " ";
        for(int i = 0; i < obj.m_operands.size(); i++) {
            os << *obj.m_operands[i];
            if(i < obj.m_operands.size() - 1)
                os << ", ";
        }
        return os;
    }
};

}