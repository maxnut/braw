#include "et_builder.hpp"

#include <unordered_map>

std::unordered_map<const AST::FileNode*, std::shared_ptr<FileNode>> s_fileCache;

std::shared_ptr<FileNode> ETBuilder::build(const AST::FileNode* node, BrawContext& context) {
    if(s_fileCache.contains(node)) return s_fileCache[node];

    std::shared_ptr<FileNode> file = std::make_shared<FileNode>();

    for(auto& import : node->m_imports)
        file->m_imports.push_back(build(import.get(), context));

    for(auto& function : node->m_functions)
        file->m_functions.push_back(build(function.get(), context));

    return file;
}