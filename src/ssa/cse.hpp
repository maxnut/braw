#pragma once

#include "ssa/block.hpp"
#include "ssa/file.hpp"
#include <memory>
#include <unordered_set>

namespace SSA {

class CSE {
public:
    static bool run(SSA::Function& file);

    static size_t hashOperation(const SSA::Operation* op);
    static size_t hashOperand(const SSA::Operand* op);

private:
    static bool run(Function& f, std::shared_ptr<Block> block, std::unordered_set<std::shared_ptr<Block>>& visited, std::unordered_map<size_t, std::shared_ptr<Register>>& expressions, std::unordered_map<std::string, std::shared_ptr<Register>>& originalToVersioned);
};

}