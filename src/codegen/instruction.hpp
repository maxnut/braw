#pragma once

#include "operand.hpp"

#include <memory>
#include <vector>

namespace CodeGen {

struct Instruction {
    const char* m_code;
    std::vector<std::shared_ptr<Operand>> m_operands;

    friend std::ostream& operator<<(std::ostream& os, const Instruction& obj) {
        os << obj.m_code << " ";
        for(int i = 0; i < obj.m_operands.size(); i++) {
            os << obj.m_operands[i];
            if(i < obj.m_operands.size() - 1)
                os << ", ";
        }
        return os;
    }
};

}