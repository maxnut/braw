#pragma once

#include "../operand.hpp"
#include "braw_context.hpp"
#include "instruction.hpp"
#include "ir/file.hpp"
#include "graph-color/graph_color.hpp"
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
    BrawContext& brawCtx;
    std::unordered_map<std::string, std::shared_ptr<Operand>> m_virtualRegisters;
    int64_t m_spills = 0;
    std::vector<std::shared_ptr<Range>> m_ranges;
    std::vector<std::shared_ptr<Operands::Register>> m_savedRegisters;
    uint64_t m_instructionIndex = 0;
    uint64_t m_functionIndex = 0;
};

class CodeGenerator {
public:
    File generate(const ::File& src, BrawContext& braw);

private:
    void initializeRegisters();

    void generate(const ::Instruction* instr, FunctionContext& ctx);

    void move(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx);
    void add(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx);
    void sub(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx);
    void mul(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx);
    void push(std::shared_ptr<Operand> target, FunctionContext& ctx);
    void pop(std::shared_ptr<Operands::Register> target, FunctionContext& ctx);
    void call(std::shared_ptr<Operands::Label> label, std::shared_ptr<Operands::Register> optReturn, const std::vector<::Operand>& args, size_t skipArgs, FunctionContext& ctx);
    void ret(FunctionContext& ctx);

    std::shared_ptr<Operands::Register> memoryValueToRegister(std::shared_ptr<Operands::Address> address, FunctionContext& ctx);
    std::shared_ptr<Operands::Register> memoryAddressToRegister(std::shared_ptr<Operands::Address> address, FunctionContext& ctx);
    std::shared_ptr<Operands::Register> memoryAddressToRegister(std::shared_ptr<Operands::Address> address, std::shared_ptr<Operands::Register> store, FunctionContext& ctx);
    void compareAndStore(std::shared_ptr<Operands::Register> reg, std::shared_ptr<Operand> op, std::shared_ptr<Operands::Register> store, InstructionOpcode setOpcode, FunctionContext& ctx);
    void compareAndJump(std::shared_ptr<Operands::Register> reg, std::shared_ptr<Operand> op, std::shared_ptr<Operands::Label> label, InstructionOpcode jumpOpcode, FunctionContext& ctx);
    std::shared_ptr<Operands::Address> copyAddressToNew(std::shared_ptr<Operands::Address> address, size_t size, FunctionContext& ctx);
    void copyAddressToAddress(std::shared_ptr<Operands::Address> target, std::shared_ptr<Operands::Address> source, size_t size, FunctionContext& ctx);
    void copyAddressToAddressPointer(std::shared_ptr<Operands::Address> target, std::shared_ptr<Operands::Address> source, size_t size, FunctionContext& ctx);


    std::shared_ptr<Operand> convertOperand(::Operand source, FunctionContext& ctx);

    bool bothAddress(std::shared_ptr<Operand> o1, std::shared_ptr<Operand> o2) const;
    bool isFloat(std::shared_ptr<Operand> o) const;
    bool isDouble(std::shared_ptr<Operand> o) const;
    bool isRegister(std::shared_ptr<Operand> o) const {return o->m_type == Operand::Type::Register;}
    bool bothRegisters(std::shared_ptr<Operand> o1, std::shared_ptr<Operand> o2) const {return isRegister(o1) && isRegister(o2);};
    bool isSmaller(std::shared_ptr<Operands::Register> op, std::shared_ptr<Operands::Register> than) const {return op->m_typeInfo.m_size < than->m_typeInfo.m_size;}

private:
    std::unordered_map<Operands::Register::RegisterGroup, std::shared_ptr<Operands::Register>> m_registers;
    std::unordered_map<std::string, std::shared_ptr<Operands::Address>> m_addresses;
};
    
}