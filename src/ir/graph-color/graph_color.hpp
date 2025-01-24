#pragma once

#include "ir/function.hpp"
#include "ir/register.hpp"

#include <cstdint>
#include <string>
#include <unordered_set>
#include <utility>
#include <unordered_map>
#include <vector>

struct Range {
    std::pair<uint32_t, uint32_t> m_range;
    RegisterType m_registerType;
};

struct GraphNode {
    std::string m_id;
    std::vector<std::string> m_connections;
    std::string m_tag = "";
};

struct ColorResult {
    std::unordered_map<std::string, std::string> m_registers;
    std::unordered_set<std::string> m_spills;
    std::unordered_map<std::string, Range> m_ranges;
};

class GraphColor {
public:
    static ColorResult build(const Function& function, std::vector<std::string> registers, std::vector<std::string> precisionRegisters);

private:
    static std::unordered_map<std::string, Range> fillRanges(const Function& function);
    static std::vector<std::string> getOverlaps(const std::string& id, const std::unordered_map<std::string, Range>& ranges);
    static void removeFromGraph(const std::string& id, std::vector<GraphNode>& graph);
    static GraphNode getMostRelevantNode(std::vector<GraphNode>& graph);
    static GraphNode findInGraph(const std::string& id, std::vector<GraphNode>& graph);
};