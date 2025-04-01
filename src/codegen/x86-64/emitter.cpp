#include "emitter.hpp"
#include "braw_context.hpp"
#include "codegen/x86-64/instruction.hpp"
#include "ir/operand.hpp"
#include "ir/printer/ir_printer.hpp"
#include <iomanip>

namespace CodeGen::x86_64 {

void opcodeInstruction(const Instruction& obj, std::ostream& os) {
    os << prefixToString(obj.m_prefix) << opcodeToString(obj.m_opcode);
}

void Emitter::emit(const File& f, const ::File& ir, std::ostream& out, const BrawContext& ctx) {
    const char* sectionPrefix = ".section";
    const char* globalPrefix = ".global";
    const char* externPrefix = ".extern";
    const char* commentPrefix = "#";
    const char* floatPrefix = ".float";
    const char* doublePrefix = ".double";
    const char* stringPrefix = ".asciz";

    out << ".section .note.GNU-stack,\"\",@progbits\n";
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
                out << stringPrefix << " " << "\"" << std::get<std::string>(value) << "\"" << "\n";
                break;
            }
            default: break;
        }
    }

    out << "\n" << sectionPrefix << " .text\n";

    for(auto& global : f.m_text.m_globals)
        out << globalPrefix << " " << global.m_id << "\n";

    for(auto& external : f.m_text.m_externals)
        out << externPrefix << " " << external.m_id << "\n";

    size_t labels = 0;
    for(uint32_t i = 0; i < f.m_text.m_instructions.size(); i++) {
        emit(f.m_text.m_instructions[i], out, ctx);
        out << "\t" << commentPrefix << " ";
        for(auto& op : f.m_text.m_instructions[i].m_operands) 
            out << op->m_typeInfo.m_name << " ";
        IRPrinter::print(out, ir.m_functions[f.m_text.m_instructions[i].m_irFunctionIndex].m_instructions[f.m_text.m_instructions[i].m_irIndex].get());
        labels++;
    }
}

void Emitter::emit(const Instruction& instr, std::ostream& out, const BrawContext& ctx) {
    if(instr.m_opcode == LabelOp) {
        emit((const Operands::Label*)instr.m_operands[0].get(), instr, out, ctx);
        out << ":";
        return;
    }
    opcodeInstruction(instr, out);
    out << " ";
    for(int i = 0; i < instr.m_operands.size(); i++) {
        // instr.m_operands[i]->emit(out, ctx);
        auto op = instr.m_operands[i];
        switch(op->m_type) {
            case Operand::Type::Register: emit((const Operands::Register*)op.get(), instr, out, ctx); break;
            case Operand::Type::Label: emit((const Operands::Label*)op.get(), instr, out, ctx); break;
            case Operand::Type::Immediate: emit((const Operands::Immediate*)op.get(), instr, out, ctx); break;
            case Operand::Type::Address: emit((const Operands::Address*)op.get(), instr, out, ctx); break;
            default: break;
        }
        if(i < instr.m_operands.size() - 1)
            out << ", ";
    }
}

void Emitter::emit(const Operands::Register* reg, const Instruction& instr, std::ostream& out, const BrawContext& ctx) {
    if(!reg->m_ids.contains(Operand::getSize(reg->m_typeInfo))) {
        out << "!err";
        return;
    }

    Operand::Size size = instr.m_opcode == Lea || instr.m_opcode == Push || instr.m_opcode == Pop ? Operand::Size::Qword : Operand::getSize(reg->m_typeInfo);
    out << reg->m_ids.at(size);
}

void Emitter::emit(const Operands::Label* label, const Instruction& instr, std::ostream& out, const BrawContext& ctx) {
    out << label->m_id;
}

void Emitter::emit(const Operands::Immediate* imm, const Instruction& instr, std::ostream& out, const BrawContext& ctx) {
    out << imm->m_value;
}

void Emitter::emit(const Operands::Address* addr, const Instruction& instr, std::ostream& out, const BrawContext& ctx) {
    Operand::Size size = instr.m_opcode == Lea ? Operand::Size::Qword : Operand::getSize(addr->m_typeInfo);
    switch(size) {
        case Operand::Size::Byte: out << "BYTE PTR "; break;
        case Operand::Size::Word: out << "WORD PTR "; break;
        case Operand::Size::Dword: out << "DWORD PTR "; break;
        case Operand::Size::Qword: out << "QWORD PTR "; break;
        case Operand::Size::Oword: out << "OWORD PTR "; break;
        case Operand::Size::Yword: out << "YWORD PTR "; break;
        default: break;
    }

    if(addr->m_base->m_type == Operand::Type::Register) {
        auto reg = std::dynamic_pointer_cast<Operands::Register>(addr->m_base);

        if(addr->m_index) {
            out << '[' << reg->m_ids.at(Operand::Size::Qword);
            out << '+' << addr->m_scale << '*' << addr->m_index->m_ids.at(Operand::Size::Qword);
            out << ']';
        }
        else {
            out << '[' << reg->m_ids.at(Operand::Size::Qword);
            if(addr->m_offset != 0)
                out << (addr->m_offset > 0 ? "+" : "") << addr->m_offset;
            out << ']';
        }
    }
    else {
        auto label = std::dynamic_pointer_cast<Operands::Label>(addr->m_base);
        out << "[rip+" << label->m_id << ']';
    }
}
}