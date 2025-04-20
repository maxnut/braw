#pragma once

#include "ir/instruction.hpp"
#include "ssa/operand.hpp"
#include "ssa/operation.hpp"
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace SSA {

struct Instruction {
    enum Type {
        Assign,
        Allocate,
        Call,
        Return,
        JumpFalse,
        JumpTrue,
        Jump,
        Label,
        Phi,
        WriteMem
    };

    Instruction(Type t, std::pair<uint32_t, uint32_t> range) : m_type(t), m_range(range) {}

    Type m_type;
    std::pair<uint32_t, uint32_t> m_range;
};

struct Assignment : Instruction {
    Assignment(std::pair<uint32_t, uint32_t> pair) : Instruction(Type::Assign, pair) {}

    std::shared_ptr<Operand> m_to;
    std::shared_ptr<Operation> m_operation;
};

struct Allocate : Instruction {
    Allocate(std::pair<uint32_t, uint32_t> pair, std::shared_ptr<Operand> to, size_t size) : Instruction(Type::Allocate, pair), m_to(to), m_size(size) {}

    std::shared_ptr<Operand> m_to;
    size_t m_size;
};

struct Call : Instruction {
    Call(std::pair<uint32_t, uint32_t> pair) : Instruction(Type::Call, pair) {}

    std::string m_id;
    std::shared_ptr<Register> m_optReturn = nullptr;
    TypeInfo m_returnType;
    std::vector<std::shared_ptr<Operand>> m_parameters;
};

struct Label : Instruction {
    Label(std::pair<uint32_t, uint32_t> pair) : Instruction(Type::Label, pair) {}
    Label(std::pair<uint32_t, uint32_t> pair, const std::string& id) : Instruction(Type::Label, pair), m_id(id) {}

    std::string m_id;
};

struct Jump : Instruction {
    Jump(Type t, std::pair<uint32_t, uint32_t> range, std::shared_ptr<SSA::Label> to, std::shared_ptr<Operand> check = nullptr) : Instruction(t, range), m_to(to), m_check(check) {}

    std::shared_ptr<SSA::Label> m_to;
    std::shared_ptr<Operand> m_check = nullptr;
};

struct Phi : Instruction {
    Phi(std::shared_ptr<Operand> to) : Instruction(Type::Phi, {}), m_to(to) {}

    std::shared_ptr<Operand> m_to;
    std::vector<std::shared_ptr<Operand>> m_operands;
    std::vector<size_t> m_placeOpAt;
};

struct WriteMem : Instruction {
    WriteMem(std::pair<uint32_t, uint32_t> range, std::shared_ptr<Operand> to, std::shared_ptr<Operand> value) : Instruction(Type::WriteMem, range), m_to(to), m_value(value) {}

    std::shared_ptr<Operand> m_to;
    std::shared_ptr<Operand> m_value;
    std::shared_ptr<Register> m_memory;
};

}