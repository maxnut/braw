#pragma once

#include "ir/instruction.hpp"
#include "ir/operand.hpp"
#include "ir/register.hpp"
#include "type_info.hpp"
#include <memory>
#include <vector>

struct CallInstruction : Instruction {
    CallInstruction() : Instruction(Type::Call) {}

    std::string m_id;
    std::shared_ptr<Register> m_optReturn = nullptr;
    TypeInfo m_returnType;
    std::vector<Operand> m_parameters;
};