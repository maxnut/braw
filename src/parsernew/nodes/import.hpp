#pragma once

#include "node.hpp"

#include <filesystem>

namespace AST {

struct ImportNode : Node {
    ImportNode() : Node(Type::Import) {}

    std::filesystem::path m_path;
};

}