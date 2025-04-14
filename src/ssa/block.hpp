#pragma once

#include "ssa/instruction.hpp"
#include <memory>
#include <utility>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace SSA {

struct Block {
    std::pair<uint32_t, uint32_t> m_instructionRange;
    std::vector<std::shared_ptr<Block>> m_connections;
    std::vector<std::shared_ptr<Block>> m_predecessors;
    std::vector<std::shared_ptr<Block>> m_dominators;
    std::vector<std::shared_ptr<Block>> m_dominated;
    std::unordered_set<std::shared_ptr<Block>> m_dominanceFrontiers;
    std::unordered_map<std::string, std::shared_ptr<SSA::Phi>> m_phiForVariable;

    std::shared_ptr<Block> getImmediateDomiator() {
        for(int i = m_dominators.size() - 1; i >= 0; i--) {
            if(m_dominators.at(i).get() == this)
                continue;
            return m_dominators.at(i);
        }
        return nullptr;
    }
};

}