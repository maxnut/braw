
#pragma once

#include "node.hpp"
#include "parser/identifier.hpp"

#include <cstdint>
#include <memory>
#include <unordered_map>

namespace AST {

struct MacroNode : Node {
    MacroNode() : Node(Type::Macro) {}

    Identifier m_name;
    std::shared_ptr<Node> m_node = nullptr;
    std::unordered_map<std::string, uint32_t> m_parameters;
};

}