#include "emitter.hpp"

namespace CodeGen::x86_64 {

void Emitter::emit(const File& f, std::ostream& out) {
    out << "section .text\n";

    for(uint32_t i = 0; i < f.m_text.m_instructions.size(); i++) {
        if(f.m_text.m_labels.contains(i))
            out << f.m_text.m_labels.at(i).m_id << ":\n";

        out << f.m_text.m_instructions[i] << "\n";
    }
}

}