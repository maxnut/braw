#include "ir_builder.hpp"

std::vector<File> IRBuilder::build(const AST::FileNode* root, BrawContext& context) {
    std::vector<File> files;

    for(auto& imp : root->m_imports) {
        auto imports = build(imp.get(), context);
        for(auto& imp2 : imports)
            files.push_back(std::move(imp2));
    }

    File file;

    for(auto& func : root->m_functions)
        file.m_functions.push_back(build(func.get(), context));

    files.push_back(std::move(file));

    return files;
}