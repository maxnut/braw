#pragma once

#include "ir/file.hpp"
#include "ir/function.hpp"
#include "ir/instructions/call.hpp"
#include "ir/instructions/basic.hpp"

#include <ostream>

class IRPrinter {
public:
    static void print(std::ostream& out, const File& file);
    static void print(std::ostream& out, const Function& function);

    static void print(std::ostream& out, const Instruction* instr);
    static void print(std::ostream& out, const BasicInstruction* instr);
    static void print(std::ostream& out, const CallInstruction* instr);
};