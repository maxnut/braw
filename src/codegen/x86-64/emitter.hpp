#pragma once

#include "braw_context.hpp"
#include "codegen/x86-64/address.hpp"
#include "codegen/x86-64/immediate.hpp"
#include "codegen/x86-64/instruction.hpp"
#include "codegen/x86-64/olabel.hpp"
#include "codegen/x86-64/register.hpp"
#include "file.hpp"
#include "ir/file.hpp"
#include <ostream>

namespace CodeGen::x86_64 {

class Emitter {
public:
    static void emit(const File& f, const ::File& ir, std::ostream& out, const BrawContext& ctx);

private:
    static void emit(const Instruction& instr, std::ostream& out, const BrawContext& ctx);
    static void emit(const Operands::Register* reg, const InstructionOpcode& instr, std::ostream& out, const BrawContext& ctx);
    static void emit(const Operands::Label* label, const InstructionOpcode& instr, std::ostream& out, const BrawContext& ctx);
    static void emit(const Operands::Immediate* imm, const InstructionOpcode& instr, std::ostream& out, const BrawContext& ctx);
    static void emit(const Operands::Address* addr, const InstructionOpcode& instr, std::ostream& out, const BrawContext& ctx);
};

}