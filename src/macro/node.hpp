#pragma once

#include "parser/nodes/node.hpp"
#include <memory>
#include <string>
#include <unordered_map>

namespace Macro {

struct Node {
    std::string m_value = "";
    std::shared_ptr<AST::Node> m_node = nullptr;
    std::unordered_map<std::string, std::shared_ptr<Node>> m_members;
};

}