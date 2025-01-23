#include "compiler.hpp"

void Compiler::compile(const Function& function, std::ofstream& fs) {
    ColorResult res = GraphColor::build(function, {"rbx", "r10", "r11", "r12", "r13", "r14", "r15"});

    size_t spillSize = res.m_spills.size() * 4; //TODO: not only dwords
    CompilerContext ctx = {std::move(res), spillSize};

    compile(function.m_instructions[0].get(), ctx, fs); //label

    fs << "push rbp\n";
    if(spillSize > 0)
        fs << "sub rsp, " << spillSize << "\n";

    for(uint32_t i = 1; i < function.m_instructions.size(); i++)
        compile(function.m_instructions[i].get(), ctx, fs);
}