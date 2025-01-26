#include "semantic_analyzer.hpp"
#include "parser/nodes/file.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::FileNode* file, BrawContext& ctx) {
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

    return errOpt;
}