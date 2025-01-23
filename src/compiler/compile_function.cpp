#include "compiler.hpp"

void Compiler::compile(const Function& function, std::ofstream& fs) {
    ColorResult res = GraphColor::build(function, {"rbx", "r10", "r11", "r12", "r13", "r14", "r15"});

    size_t spillSize = res.m_spills.size() * 4; //TODO: not only dwords

    fs << "push rbp\n";
    if(spillSize > 0)
        fs << "sub rsp, " << spillSize << "\n";

    for(auto& instr : function.m_instructions)
        compile(instr.get(), res, fs);

    if(spillSize > 0)
        fs << "add rsp, " << spillSize << "\n";

    fs << "pop rbp\nret\n";
}