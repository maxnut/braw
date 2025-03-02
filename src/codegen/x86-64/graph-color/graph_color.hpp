#pragma once

#include "ir/function.hpp"
#include "ir/register.hpp"
#include "../register.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <unordered_map>
#include <vector>

namespace CodeGen::x86_64 {

struct Range {
    std::string m_id;
    std::pair<uint32_t, uint32_t> m_range;
    RegisterType m_registerType;
    TypeInfo m_typeInfo;
    bool m_isPointedOrDereferenced = false;
};

struct GraphNode {
    std::string m_id;
    std::vector<std::string> m_connections;
    Operands::Register::RegisterGroup m_tag = Operands::Register::Count;
    RegisterType m_registerType;
};

struct ColorResult {
    std::unordered_map<std::string, Operands::Register::RegisterGroup> m_registers;
    std::unordered_set<std::string> m_spills;
    std::unordered_map<std::string, std::shared_ptr<Range>> m_ranges;
    std::vector<std::shared_ptr<Range>> m_rangeVector;
};

class GraphColor {
public:
    static ColorResult build(const Function& function, std::vector<Operands::Register::RegisterGroup> registers, std::vector<Operands::Register::RegisterGroup> precisionRegisters, int maxParamReg, int maxParamPReg);

private:
    static void fillRanges(const Function& function, ColorResult& result);
    static std::vector<std::string> getOverlaps(const std::string& id, const std::unordered_map<std::string, std::shared_ptr<Range>>& ranges);
    static void removeFromGraph(const std::string& id, std::vector<GraphNode>& graph);
    static GraphNode getMostRelevantNode(std::vector<GraphNode>& graph);
    static GraphNode findInGraph(const std::string& id, std::vector<GraphNode>& graph);
};

}