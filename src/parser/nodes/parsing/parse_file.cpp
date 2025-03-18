#include "parser/parser.hpp"
#include "../file.hpp"
#include <memory>
#include <unordered_map>

Result<std::shared_ptr<AST::FileNode>> Parser::parseFile(TokenCursor& cursor, std::filesystem::path path) {
    static std::unordered_map<std::filesystem::path, std::shared_ptr<AST::FileNode>> importCache;
    if(importCache.contains(path))
        return importCache[path];
    
    std::shared_ptr<AST::FileNode> file = std::make_shared<AST::FileNode>();
    file->m_path = path;
    file->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    
    while(cursor.hasNext()) {
        if(Rules::isFunctionDefinition(cursor)) {
            auto function = parseFunctionDefinition(cursor, path);
            
            if(!function)
                return std::unexpected{function.error()};

            file->m_functions.push_back(std::move(function.value()));
            continue;
        }
        else if(Rules::isStructDefinition(cursor)) {
            auto type = parseStructDefinition(cursor, path);

            if(!type)
                return std::unexpected{type.error()};

            file->m_structs.push_back(std::move(type.value()));
            continue;
        }
        else if(Rules::isImport(cursor)) {
            auto import = parseImport(cursor, path);

            if(!import)
                return std::unexpected{import.error()};

            file->m_imports.push_back(import.value());
            continue;
        }

        return unexpectedToken(cursor.get().value(), path);
    }

    file->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    importCache[path] = file;
    return file;
}