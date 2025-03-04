#pragma once

#include "braw_context.hpp"
#include "file.hpp"
#include "ir/file.hpp"
#include <ostream>

namespace CodeGen::x86_64 {

class Emitter {
public:
    static void emit(const File& f, const ::File& ir, std::ostream& out, const BrawContext& ctx);

private:
    static void emit(const Instruction& instr, std::ostream& out, const BrawContext& ctx);
};

}