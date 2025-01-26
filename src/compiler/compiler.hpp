#pragma once

#include "ir/file.hpp"
#include "ir/instruction.hpp"
#include "ir/instructions/binary.hpp"
#include "ir/graph-color/graph_color.hpp"
#include "ir/instructions/call.hpp"
#include "ir/value.hpp"

#include <filesystem>
#include <fstream>
#include <unordered_map>

typedef std::unordered_map<Value, std::string> LiteralPool;

struct CompilerContext {
    ColorResult m_reg;
    size_t m_spillSize = 0;
    size_t m_instructionIndex = 0;
    LiteralPool& m_literalPool;

    std::string getLiteralLabel(Value value);
};

class Compiler {
public:
    static void compile(const File& file, std::filesystem::path out);

private:
    static void compile(const Function& function, LiteralPool& literalPool, std::ofstream& fs);

    static void compile(const Instruction* instr, CompilerContext& ctx, std::ofstream& fs);
    static void compile(const BinaryInstruction* instr, CompilerContext& ctx, std::ofstream& fs);
    static void compile(const CallInstruction* instr, CompilerContext& ctx, std::ofstream& fs);
    static void compileReturn(const Instruction* instr, CompilerContext& ctx, std::ofstream& fs);
};