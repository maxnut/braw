#include "semantic_analyzer.hpp"
#include "parser/nodes/file.hpp"
#include <unordered_set>

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::FileNode* file, BrawContext& ctx) {
    static std::unordered_set<std::filesystem::path> analyzed;
    if(analyzed.contains(file->m_path)) return std::nullopt;
    analyzed.insert(file->m_path);

    std::filesystem::path prev = ctx.m_currentFile;
    ctx.m_currentFile = file->m_path;

    std::optional<SemanticError> errOpt;

    for(auto& import : file->m_imports) {
        errOpt = analyze(import.get(), ctx);
        if(errOpt) return errOpt;
    }

    for(auto& struct_ : file->m_structs) {
        errOpt = analyze(struct_.get(), ctx);
        if(errOpt) return errOpt;
    }

    for(auto& function : file->m_functions) {
        errOpt = analyze(function.get(), ctx);
        if(errOpt) return errOpt;
    }

    ctx.m_currentFile = prev;
    return errOpt;
}