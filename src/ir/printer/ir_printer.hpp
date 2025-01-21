#pragma once

#include "ir/file.hpp"
#include "ir/function.hpp"
#include "ir/label.hpp"
#include "ir/instructions/binary.hpp"
#include "ir/instructions/unary.hpp"

#include <ostream>

class IRPrinter {
public:
    static void print(std::ostream& out, const File& file);
    static void print(std::ostream& out, const Function& function);

    static void print(std::ostream& out, const BinaryInstruction* instr);
};