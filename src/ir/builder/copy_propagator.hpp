#pragma once

#include "ir/function.hpp"
#include "ir/operand.hpp"
#include <unordered_map>
#include <unordered_set>
#include <utility>

struct Block {
    std::pair<uint32_t, uint32_t> m_instructionRange;
};

class CopyPropagator {
public:
    static void propagate(Function& f);
    static std::vector<Block> getBlocks(const Function& f);
    static std::unordered_map<std::string, std::unordered_set<size_t>> getModificationPoints(const Function& f);
    static bool replace(Function& f, const std::unordered_set<size_t>& points, size_t from, size_t orBreak, const std::string& replaceId, Operand replaceWith);
};