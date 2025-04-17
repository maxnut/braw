#pragma once

#include "ssa/block.hpp"
#include "ssa/file.hpp"
#include "ssa/operand.hpp"

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace SSA {

class CopyPropagator {
public:
    static bool propagate(Function& function);
    static bool replace(std::shared_ptr<Block> block, Function& f, size_t start, std::shared_ptr<Register> replace, std::shared_ptr<Operand> with, std::unordered_set<std::shared_ptr<Block>>& visited, bool& doErase);
};

}