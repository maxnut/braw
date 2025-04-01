#pragma once

#include "ir/instruction.hpp"
#include "ir/register.hpp"
#include "ir/function.hpp"
#include "../register.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace CodeGen::x86_64 {

struct Range {
    std::string m_id;
    std::pair<uint32_t, uint32_t> m_range;
    RegisterType m_registerType;
    TypeInfo m_typeInfo;
    bool m_isPointedOrDereferenced = false;
    bool m_isAssignedFirst = false;
    Operands::Register::RegisterGroup m_forceTag = Operands::Register::Count;
    size_t m_scale = 1;
};

struct Block {
    std::pair<uint32_t, uint32_t> m_instructionRange;
    std::unordered_map<std::string, std::shared_ptr<Range>> m_ranges;
    std::vector<std::shared_ptr<Range>> m_rangeVector;
    std::vector<std::shared_ptr<Block>> m_connections;
};

struct PropagatorResult {
    std::shared_ptr<Block> root;
    std::unordered_map<size_t, size_t> blockForInstruction;
    std::vector<std::shared_ptr<Block>> blocks;
};

class Propagator {
public:
    static PropagatorResult buildGraph(const Function& f);
    static void buildGraphRecursive(std::shared_ptr<Block> root, const std::unordered_map<size_t, size_t>& blockForInstruction, const std::vector<std::shared_ptr<Block>>& blocks, std::unordered_set<std::shared_ptr<Block>>& visited, const Function& f);
    static void fillRanges(const Function& function, Block* result);
    static void visit(std::shared_ptr<Block> root, std::unordered_set<std::shared_ptr<Block>>& visited);
    static void fillHoles(std::shared_ptr<Block> from, std::shared_ptr<Block> current, std::vector<std::shared_ptr<Block>>& path, std::unordered_set<std::shared_ptr<Block>>& visited);
    static void propagate(std::shared_ptr<Block> root, std::unordered_set<std::shared_ptr<Block>>& visited);
};

}