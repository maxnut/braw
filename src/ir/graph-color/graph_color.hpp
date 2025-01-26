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
    std::string m_id;
    std::pair<uint32_t, uint32_t> m_range;
    RegisterType m_registerType;

    uint8_t size() {
        switch(m_registerType) {
            case Byte: return 1;
            case Word: return 2;
            case Dword: return 4;
            case Qword: return 8;
            case Single: return 4;
            case Double: return 8;
            default:
                break;
        }

        return 0;
    }
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
    std::vector<Range*> m_rangeVector;
};

class GraphColor {
public:
    static ColorResult build(const Function& function, std::vector<std::string> registers, std::vector<std::string> precisionRegisters);

private:
    static void fillRanges(const Function& function, ColorResult& result);
    static std::vector<std::string> getOverlaps(const std::string& id, const std::unordered_map<std::string, Range>& ranges);
    static void removeFromGraph(const std::string& id, std::vector<GraphNode>& graph);
    static GraphNode getMostRelevantNode(std::vector<GraphNode>& graph);
    static GraphNode findInGraph(const std::string& id, std::vector<GraphNode>& graph);
};