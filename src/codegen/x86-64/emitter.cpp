#include "emitter.hpp"
#include "braw_context.hpp"
#include "ir/printer/ir_printer.hpp"
#include <iomanip>

namespace CodeGen::x86_64 {

void opcodeInstruction(const Instruction& obj, std::ostream& os) {
    auto it = s_prefixes.find(obj.m_opcode.prefix);
    if (it != s_prefixes.end()) {
        os << it->second << " ";
    }

    auto it2 = opcodeMap.find(obj.m_opcode);
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
                out << "dd " << std::fixed << std::setprecision(std::numeric_limits<float>::max_digits10)
                    << std::get<float>(value) << "\n";
                break;
            case 3: {
                out << "dq " << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10)
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
        IRPrinter::print(out, ir.m_functions[f.m_text.m_instructions[i].m_irFunctionIndex].m_instructions[f.m_text.m_instructions[i].m_irIndex].get());
        labels++;
    }
}

void Emitter::emit(const Instruction& instr, std::ostream& out, const BrawContext& ctx) {
    opcodeInstruction(instr, out);
    out << " ";
    for(int i = 0; i < instr.m_operands.size(); i++) {
        instr.m_operands[i]->emit(out, ctx);
        if(i < instr.m_operands.size() - 1)
            out << ", ";
    }
}

}