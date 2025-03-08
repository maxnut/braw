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
        // assert(op->m_typeInfo.m_name != "");
        m_operands.push_back(op->clone());
    }
};

}