#include "compiler.hpp"

void Compiler::compileReturn(const Instruction* instr, CompilerContext& ctx, std::ofstream& fs) {
    if(ctx.m_spillSize > 0)
        fs << "add rsp, " << ctx.m_spillSize << "\n";
    fs << "pop rbp\nret\n";
}