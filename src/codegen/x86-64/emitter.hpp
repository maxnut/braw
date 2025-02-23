#pragma once

#include "file.hpp"
#include "ir/file.hpp"
#include <ostream>

namespace CodeGen::x86_64 {

class Emitter {
public:
    static void emit(const File& f, const ::File& ir, std::ostream& out);
};

}