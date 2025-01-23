#include "compiler.hpp"

void Compiler::compile(const Function& function, std::ofstream& fs) {
    ColorResult res = GraphColor::build(function, {"rdi", "rsi", "rdx", "rcx", "r8", "r9",
        "rbx", "r10", "r11", "r12", "r13", "r14", "r15"},
        {"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"});

    size_t spillSize = res.m_spills.size() * 4; //TODO: not only dwords
    CompilerContext ctx = {std::move(res), spillSize};

    fs << "\n";

    compile(function.m_instructions[0].get(), ctx, fs); //label

    fs << "push rbp\n";
    if(spillSize > 0)
        fs << "sub rsp, " << spillSize << "\n";

    for(uint32_t i = 1; i < function.m_instructions.size(); i++) {
        ctx.m_instructionIndex = i;
        compile(function.m_instructions[i].get(), ctx, fs);
    }
}