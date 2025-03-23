#pragma once

#include "function_definition.hpp"
#include "parser/nodes/node.hpp"
#include "struct.hpp"

#include <filesystem>
#include <unordered_map>
#include <vector>
#include <memory>

namespace AST {

struct FileNode : Node {
    FileNode() : Node(Type::File) {}

    std::vector<std::shared_ptr<FunctionDefinitionNode>> m_functions;
    std::vector<std::shared_ptr<StructNode>> m_structs;
    std::vector<std::shared_ptr<FileNode>> m_imports;
    std::unordered_map<std::string, std::shared_ptr<Node>> m_defines;
    std::filesystem::path m_path;
};

}