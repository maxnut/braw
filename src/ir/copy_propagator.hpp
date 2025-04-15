#pragma once

#include "ir/function.hpp"
#include "ir/operand.hpp"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>

struct Block {
    std::pair<uint32_t, uint32_t> m_instructionRange;
    std::vector<std::shared_ptr<Block>> m_connections;
};

class CopyPropagator {
public:
    static void propagate(Function& f);
    static void buildGraphRecursive(std::shared_ptr<Block> root, const std::unordered_map<size_t, size_t>& blockForInstruction, const std::vector<std::shared_ptr<Block>>& blocks, std::unordered_set<std::shared_ptr<Block>>& visited, const Function& f);
    static std::vector<std::shared_ptr<Block>> getBlocks(const Function& f);
    static std::unordered_map<std::string, std::unordered_set<size_t>> getModificationPoints(const Function& f);
    static bool replace(Function& f, size_t from, std::shared_ptr<Block> block, const std::string& replaceId, Operand replaceWith, std::unordered_set<std::shared_ptr<Block>>& visited);
    static bool checkReassign(Function& f, const std::unordered_set<size_t>& points, size_t from, std::shared_ptr<Block> block, std::unordered_set<std::shared_ptr<Block>>& visited);
};