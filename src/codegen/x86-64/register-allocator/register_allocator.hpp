#pragma once

#include "propagator.hpp"

#include <memory>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace CodeGen::x86_64 {

struct GraphNode {
    std::string m_id;
    std::vector<std::string> m_connections;
    Operands::Register::RegisterGroup m_tag = Operands::Register::Count;
    RegisterType m_registerType;
};

struct RegisterAllocatorResult {
    std::unordered_map<std::string, Operands::Register::RegisterGroup> m_registers;
    std::unordered_set<std::string> m_spills;
    PropagatorResult m_propagated;
};

class RegisterAllocator {
public:
    static RegisterAllocatorResult build(const Function& function, std::vector<Operands::Register::RegisterGroup> registers, std::vector<Operands::Register::RegisterGroup> precisionRegisters, int maxParamReg, int maxParamPReg);

private:
    static std::vector<std::string> getOverlaps(const std::string& id, const std::unordered_map<std::string, std::shared_ptr<Range>>& ranges);
    static void removeFromGraph(const std::string& id, std::vector<GraphNode>& graph);
    static GraphNode getMostRelevantNode(std::vector<GraphNode>& graph);
    static GraphNode findInGraph(const std::string& id, std::vector<GraphNode>& graph);
};

}