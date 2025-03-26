
#pragma once

#include "node.hpp"
#include "parser/identifier.hpp"
#include "parser/nodes/scope.hpp"

#include <cstdint>
#include <memory>
#include <unordered_map>

namespace AST {

struct MacroNode : Node {
    MacroNode() : Node(Type::Macro) {}
    MacroNode(const std::string& name, std::shared_ptr<ScopeNode> node, const std::vector<std::string>& parameters) : Node(Type::Macro), m_name(name), m_node(node), m_parameters(parameters) {}

    std::string m_name;
    std::shared_ptr<ScopeNode> m_node = nullptr;
    std::vector<std::string> m_parameters;
};

}