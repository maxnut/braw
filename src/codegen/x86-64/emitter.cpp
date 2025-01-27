#include "emitter.hpp"

namespace CodeGen::x86_64 {

void Emitter::emit(const File& f, std::ostream& out) {
    out << "bits 64\n";

    out << "section .data\n";

    for(const auto& pair : f.m_data.m_labels) {
        out << pair.first << ":\n";
        Value value = pair.second;

        switch(value.index()) {
            case 2:
                out << "dd " << std::bit_cast<unsigned int>(std::get<float>(value)) << "\n";
                break;
            case 3: {
                uint32_t lower, upper;
                memcpy(&lower, &std::get<double>(value), sizeof(lower));
                memcpy(&upper, &std::get<double>(value) + sizeof(lower), sizeof(upper));
                out << "dd " << lower;
                out << ", " << upper << "\n";
                break;
            }
            case 5: {
                out << "dd " << std::get<std::string>(value) << "\n";
                break;
            }
            default: break;
        }
    }

    out << "section .text\n";

    for(uint32_t i = 0; i < f.m_text.m_instructions.size(); i++) {
        if(f.m_text.m_labels.contains(i))
            out << f.m_text.m_labels.at(i).m_id << ":\n";

        out << f.m_text.m_instructions[i] << "\n";
    }
}

}