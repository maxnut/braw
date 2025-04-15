#pragma once

#include "braw_context.hpp"
#include "ir/address.hpp"
#include "ir/file.hpp"
#include "ir/operand.hpp"
#include "ssa/block.hpp"
#include "ssa/file.hpp"
#include "ssa/instruction.hpp"
#include "ssa/operand.hpp"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct IRFunctionContextSSA {
    BrawContext& ctx;
    std::unordered_map<std::string, std::shared_ptr<Register>> m_registers;
    std::unordered_map<std::string, Address> m_refs;
    std::unordered_map<size_t, size_t> m_ssaToIrIdx;
    Function* m_function;
};

class IRBuilderSSA {
public:
    static File build(const SSA::File& file, BrawContext& context);

private:
    static Function build(const SSA::Function& function, BrawContext& context);
    static void build(const SSA::Instruction* instruction, IRFunctionContextSSA& context);
    static void build(const SSA::Assignment* assignment, IRFunctionContextSSA& context);
    static void build(const SSA::Allocate* allocate, IRFunctionContextSSA& context);
    static void build(const SSA::Call* call, IRFunctionContextSSA& context);
    static void build(const SSA::Jump* jump, IRFunctionContextSSA& context);
    static void build(const SSA::Label* label, IRFunctionContextSSA& context);
    static void build(const SSA::Phi* phi, IRFunctionContextSSA& context);
    static void build(const SSA::WriteMem* writeMem, IRFunctionContextSSA& context);

    static Operand convertOperand(const SSA::Operand* operand, IRFunctionContextSSA& context);
    static RegisterType getRegisterType(const TypeInfo& type);
    static TypeInfo getOperandType(Operand op, IRFunctionContextSSA& ictx);
    static void moveToRegister(const std::string& name, Operand& op, std::pair<uint32_t, uint32_t> pos, IRFunctionContextSSA& ictx);
    static std::shared_ptr<Register> makeOrGetRegister(const std::string& name, IRFunctionContextSSA& ictx);
    static void buildAssignment(const SSA::Assignment* assignment, Instruction::Type type, IRFunctionContextSSA& ictx);
    static void buildCompare(const SSA::Assignment* assignment, Instruction::Type type, IRFunctionContextSSA& ictx);
};