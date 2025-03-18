#include "ir_builder.hpp"
#include <unordered_map>

std::vector<File> IRBuilder::build(const AST::FileNode* root, BrawContext& context) {
    static std::unordered_set<std::filesystem::path> imported;
    std::vector<File> files;

    for(auto& imp : root->m_imports) {
        if(imported.contains(imp->m_path)) continue;
        imported.insert(imp->m_path);
        auto imports = build(imp.get(), context);
        for(auto& imp2 : imports) {
            if(imp2.m_functions.size() <= 0) continue;
            files.push_back(std::move(imp2));
        }
    }

    File file;
    file.m_path = root->m_path;

    file.m_functions.reserve(root->m_functions.size()); // avoid vector reallocation
    for(auto& func : root->m_functions) {
        file.m_functions.push_back(build(func.get(), context));
        if(file.m_functions.back().m_external)
            file.m_externals.push_back(&file.m_functions.back());
    }

    for(File& f : files) {
        for(Function& fn : f.m_functions)
            file.m_externals.push_back(&fn);
    }

    files.push_back(std::move(file));

    return files;
}