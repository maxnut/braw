#pragma once

#include "ir/file.hpp"
#include "ir/function.hpp"
#include "ir/instructions/call.hpp"
#include "ir/instructions/binary.hpp"

#include <ostream>

class IRPrinter {
public:
    static void print(std::ostream& out, const File& file);
    static void print(std::ostream& out, const Function& function);

    static void print(std::ostream& out, const BinaryInstruction* instr);
    static void print(std::ostream& out, const CallInstruction* instr);
};