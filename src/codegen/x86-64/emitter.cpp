#include "emitter.hpp"
#include "braw_context.hpp"
#include "codegen/x86-64/instruction.hpp"
#include "ir/operand.hpp"
#include "ir/printer/ir_printer.hpp"
#include <iomanip>

namespace CodeGen::x86_64 {

void opcodeInstruction(const Instruction& obj, std::ostream& os) {
    if(!obj.m_opcode.usesPrefix) {
        auto it = s_prefixes.find(obj.m_opcode.prefix);
        if (it != s_prefixes.end()) {
            os << it->second << " ";
        }
    }

    InstructionOpcode find = obj.m_opcode;
    if(!obj.m_opcode.usesPrefix)
        find.prefix = 0;
    auto it2 = opcodeMap.find(find);
    if (it2 != opcodeMap.end()) {
        os << it2->second;
    } else {
        os << "unknown_opcode";
    }
}

void Emitter::emit(const File& f, const ::File& ir, std::ostream& out, const BrawContext& ctx) {
    const char* sectionPrefix = ctx.m_assembler == NASM ? "section" : ".section";
    const char* globalPrefix = ctx.m_assembler == NASM ? "global" : ".global";
    const char* commentPrefix = ctx.m_assembler == NASM ? ";" : "#";
    const char* floatPrefix = ctx.m_assembler == NASM ? "dd" : ".float";
    const char* doublePrefix = ctx.m_assembler == NASM ? "dd" : ".double";

    if(ctx.m_assembler == NASM)
        out << "bits 64\n\n";
    else if(ctx.m_assembler == GAS)
        out << ".intel_syntax noprefix\n\n";

    out << sectionPrefix << " .data\n";

    for(const auto& pair : f.m_data.m_labels) {
        out << pair.first << ":\n";
        Value value = pair.second;

        switch(value.index()) {
            case 2:
                out << floatPrefix << " " << std::fixed << std::setprecision(std::numeric_limits<float>::max_digits10)
                    << std::get<float>(value) << "\n";
                break;
            case 3: {
                out << doublePrefix << " " << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10)
                    << std::get<double>(value) << "\n";
                break;
            }
            case 5: {
                out << "dd " << std::get<std::string>(value) << "\n";
                break;
            }
            default: break;
        }
    }

    out << "\n" << sectionPrefix << " .text\n";

    for(auto& global : f.m_text.m_globals) {
        out << globalPrefix << " " << global.m_id << "\n";
    }

    size_t labels = 0;
    for(uint32_t i = 0; i < f.m_text.m_instructions.size(); i++) {
        while(f.m_text.m_labels.contains(labels))
            out << f.m_text.m_labels.at(labels++).m_id << ":\n";

        emit(f.m_text.m_instructions[i], out, ctx);
        out << "\t" << commentPrefix << " ";
        for(auto& op : f.m_text.m_instructions[i].m_operands) 
            out << op->m_typeInfo.m_name << " ";
        IRPrinter::print(out, ir.m_functions[f.m_text.m_instructions[i].m_irFunctionIndex].m_instructions[f.m_text.m_instructions[i].m_irIndex].get());
        labels++;
    }
}

void Emitter::emit(const Instruction& instr, std::ostream& out, const BrawContext& ctx) {
    opcodeInstruction(instr, out);
    out << " ";
    for(int i = 0; i < instr.m_operands.size(); i++) {
        // instr.m_operands[i]->emit(out, ctx);
        auto op = instr.m_operands[i];
        switch(op->m_type) {
            case Operand::Type::Register: emit((const Operands::Register*)op.get(), instr.m_opcode, out, ctx); break;
            case Operand::Type::Label: emit((const Operands::Label*)op.get(), instr.m_opcode, out, ctx); break;
            case Operand::Type::Immediate: emit((const Operands::Immediate*)op.get(), instr.m_opcode, out, ctx); break;
            case Operand::Type::Address: emit((const Operands::Address*)op.get(), instr.m_opcode, out, ctx); break;
            default: break;
        }
        if(i < instr.m_operands.size() - 1)
            out << ", ";
    }
}

void Emitter::emit(const Operands::Register* reg, const InstructionOpcode& instr, std::ostream& out, const BrawContext& ctx) {
    if(!reg->m_ids.contains(Operand::getSize(reg->m_typeInfo))) {
        out << "!err";
        return;
    }

    Operand::Size size = instr == Push || instr == Pop ? Operand::Size::Qword : Operand::getSize(reg->m_typeInfo);
    out << reg->m_ids.at(size);
}

void Emitter::emit(const Operands::Label* label, const InstructionOpcode& instr, std::ostream& out, const BrawContext& ctx) {
    out << label->m_id;
}

void Emitter::emit(const Operands::Immediate* imm, const InstructionOpcode& instr, std::ostream& out, const BrawContext& ctx) {
    out << imm->m_value;
}

void Emitter::emit(const Operands::Address* addr, const InstructionOpcode& instr, std::ostream& out, const BrawContext& ctx) {
    Operand::Size size = instr == Lea ? Operand::Size::Qword : Operand::getSize(addr->m_typeInfo);
    switch(size) {
        case Operand::Size::Byte: out << (ctx.m_assembler == GAS ? "BYTE PTR " : "byte "); break;
        case Operand::Size::Word: out << (ctx.m_assembler == GAS ? "WORD PTR " : "word "); break;
        case Operand::Size::Dword: out << (ctx.m_assembler == GAS ? "DWORD PTR " : "dword "); break;
        case Operand::Size::Qword: out << (ctx.m_assembler == GAS ? "QWORD PTR " : "qword "); break;
        case Operand::Size::Oword: out << (ctx.m_assembler == GAS ? "OWORD PTR " : "oword "); break;
        case Operand::Size::Yword: out << (ctx.m_assembler == GAS ? "YWORD PTR " : "yword "); break;
        default: break;
    }

    if(addr->m_base->m_type == Operand::Type::Register) {
        auto reg = std::dynamic_pointer_cast<Operands::Register>(addr->m_base);

        out << '[' << reg->m_ids.at(Operand::Size::Qword);
        if(addr->m_offset != 0)
            out << (addr->m_offset > 0 ? "+" : "") << addr->m_offset;
        out << ']';
    }
    else {
        auto label = std::dynamic_pointer_cast<Operands::Label>(addr->m_base);
        out << "[rip+" << label->m_id << ']';
    }
}
}