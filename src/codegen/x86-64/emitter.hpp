#pragma once

#include "file.hpp"
#include <ostream>

namespace CodeGen::x86_64 {

class Emitter {
public:
    static void emit(const File& f, std::ostream& out);
};

}