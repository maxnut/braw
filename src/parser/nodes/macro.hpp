
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

    Identifier m_name;
    std::shared_ptr<ScopeNode> m_node = nullptr;
    std::vector<Identifier> m_parameters;
};

}