#pragma once

#include "ir/file.hpp"
#include "ir/instruction.hpp"
#include "ir/instructions/binary.hpp"
#include "ir/graph-color/graph_color.hpp"

#include <filesystem>
#include <fstream>

class Compiler {
public:
    static void compile(const File& file, std::filesystem::path out);

private:
    static void compile(const Function& function, std::ofstream& fs);

    static void compile(const Instruction* instr, const ColorResult& reg, std::ofstream& fs);
    static void compile(const BinaryInstruction* instr, const ColorResult& reg, std::ofstream& fs);
};