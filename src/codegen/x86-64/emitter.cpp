#include "emitter.hpp"
#include "ir/printer/ir_printer.hpp"
#include <iomanip>

namespace CodeGen::x86_64 {

void Emitter::emit(const File& f, const ::File& ir, std::ostream& out) {
    out << "bits 64\n\n";

    out << "section .data\n";

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

    out << "\nsection .text\n";

    for(auto& global : f.m_text.m_globals) {
        out << "global " << global.m_id << "\n";
    }

    size_t labels = 0;
    for(uint32_t i = 0; i < f.m_text.m_instructions.size(); i++) {
        while(f.m_text.m_labels.contains(labels))
            out << f.m_text.m_labels.at(labels++).m_id << ":\n";

        out << f.m_text.m_instructions[i] << "\t; ";
        IRPrinter::print(out, ir.m_functions[f.m_text.m_instructions[i].m_irFunctionIndex].m_instructions[f.m_text.m_instructions[i].m_irIndex].get());
        labels++;
    }
}

}