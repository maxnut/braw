#pragma once

#include "ir/file.hpp"
#include "ir/instruction.hpp"
#include "ir/instructions/binary.hpp"
#include "ir/graph-color/graph_color.hpp"
#include "ir/instructions/call.hpp"

#include <filesystem>
#include <fstream>

struct CompilerContext {
    ColorResult m_reg;
    size_t m_spillSize = 0;
    size_t m_instructionIndex = 0;
};

class Compiler {
public:
    static void compile(const File& file, std::filesystem::path out);

private:
    static void compile(const Function& function, std::ofstream& fs);

    static void compile(const Instruction* instr, const CompilerContext& ctx, std::ofstream& fs);
    static void compile(const BinaryInstruction* instr, const CompilerContext& ctx, std::ofstream& fs);
    static void compile(const CallInstruction* instr, const CompilerContext& ctx, std::ofstream& fs);
    static void compileReturn(const Instruction* instr, const CompilerContext& ctx, std::ofstream& fs);
};