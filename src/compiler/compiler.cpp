#include "compiler.hpp"
#include "ir/function.hpp"

void compileLiteral(Value value, std::ofstream& fs) {
    switch(value.index()) {
        case 2:
            fs << "dd " << std::bit_cast<unsigned int>(std::get<float>(value)) << "\n";
            break;
        case 3: {
            uint32_t lower, upper;
            memcpy(&lower, &std::get<double>(value), sizeof(lower));
            memcpy(&upper, &std::get<double>(value) + sizeof(lower), sizeof(upper));
            fs << "dd " << lower;
            fs << ", " << upper << "\n";
            break;
        }
        default:
            break;
    }
}

void Compiler::compile(const File& file, std::filesystem::path out) {
    std::ofstream fs(out);

    fs << "section .text\n";

    for(const Function& f : file.m_functions)
        fs << "global " << f.m_name << "\n";

    LiteralPool literalPool;

    for(const Function& f : file.m_functions)
        compile(f, literalPool, fs);

    fs << "\nsection .data\n";

    for(auto& entry : literalPool) {
        fs << entry.second << ": ";
        compileLiteral(entry.first, fs);
    }

    fs.close();
}

std::string CompilerContext::getLiteralLabel(Value value) {
    if(m_literalPool.contains(value))
        return m_literalPool.at(value);

    std::string label = "lit_" + std::to_string(m_literalPool.size());
    m_literalPool[value] = label;
    return label;
}