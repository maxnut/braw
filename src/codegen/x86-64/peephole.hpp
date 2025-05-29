#pragma once

#include "codegen/x86-64/file.hpp"
#include "codegen/x86-64/instruction.hpp"
#include "cursor.hpp"
#include <functional>
#include <span>
#include <vector>

namespace CodeGen::x86_64 {

using InstructionCursor = Cursor<std::vector<Instruction>::iterator>;
using MatcherFunction = std::function<std::span<Instruction>(InstructionCursor)>;
using ReplaceFunction = std::function<std::vector<Instruction>(std::span<Instruction>)>;

class Peephole {
public:
    static bool run(File& file);
};

}