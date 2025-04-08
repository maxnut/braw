#pragma once

#include "ssa/file.hpp"
#include "ssa/operand.hpp"
#include "ssa/operation.hpp"
#include <ostream>

namespace SSA {

class Printer {
public:
    static void print(std::ostream& out, const File& file);
    static void print(std::ostream& out, const Function& function);

    static void print(std::ostream& out, const Instruction* instr);
    static void print(std::ostream& out, const Operation* instr);
    static void print(std::ostream& out, const Operand* instr);
};

}