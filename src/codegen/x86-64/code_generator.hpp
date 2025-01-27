#pragma once

#include "../operand.hpp"
#include "instruction.hpp"
#include "ir/file.hpp"
#include "ir/operand.hpp"
#include "olabel.hpp"
#include "register.hpp"
#include "address.hpp"
#include "file.hpp"

#include <cstdint>
#include <memory>
#include <unordered_map>

namespace CodeGen::x86_64 {

struct FunctionContext {
    File& f;
    std::unordered_map<std::string, std::shared_ptr<Operand>> m_virtualRegisters;
    size_t m_spills = 0;
};

class CodeGenerator {
public:
    CodeGenerator() {initializeRegisters();}
    File generate(const ::File& src);

private:
    void initializeRegisters();

    void generate(const ::Instruction* instr, FunctionContext& ctx);

    void move(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx);
    void add(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx);
    void sub(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx);
    void mul(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx);
    void push(std::shared_ptr<Operand> target, FunctionContext& ctx);
    void pop(std::shared_ptr<Operand> target, FunctionContext& ctx);
    void ret(FunctionContext& ctx);

    std::shared_ptr<Operands::Register> memoryToRegister(std::shared_ptr<Operands::Address> address, FunctionContext& ctx);
    void compareAndStore(std::shared_ptr<Operands::Register> reg, std::shared_ptr<Operand> op, Opcode setOpcode, FunctionContext& ctx);
    void compareAndJump(std::shared_ptr<Operands::Register> reg, std::shared_ptr<Operand> op, std::shared_ptr<Operands::Label> label, Opcode jumpOpcode, FunctionContext& ctx);

    std::shared_ptr<Operand> convertOperand(::Operand source, FunctionContext& ctx);

    bool bothAddress(std::shared_ptr<Operand> o1, std::shared_ptr<Operand> o2) const;
    bool isFloat(std::shared_ptr<Operand> o) const {return o->m_valueType == Operand::ValueType::SinglePrecision;}
    bool isRegister(std::shared_ptr<Operand> o) const {return o->m_type == Operand::Type::Register;}
    bool bothRegisters(std::shared_ptr<Operand> o1, std::shared_ptr<Operand> o2) const {return isRegister(o1) && isRegister(o2);};
    bool isSmaller(std::shared_ptr<Operands::Register> op, std::shared_ptr<Operands::Register> than) const {return op->m_size < than->m_size;}

private:
    std::unordered_map<std::string, std::shared_ptr<Operands::Register>> m_registers;
};
    
}