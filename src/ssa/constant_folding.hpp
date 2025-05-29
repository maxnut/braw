#pragma once

#include "ssa/file.hpp"
#include "ssa/operand.hpp"
#include <memory>

namespace SSA {

class ConstantFolding {
public:
    static bool run(SSA::Function& fun);

private:
    static std::shared_ptr<Immediate> fold(const SSA::Operation* op);
};

}