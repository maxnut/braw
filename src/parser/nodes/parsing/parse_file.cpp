#include "parser/parser.hpp"
#include "../file.hpp"

Result<std::unique_ptr<AST::FileNode>> Parser::parseFile(TokenCursor& cursor) {
    std::unique_ptr<AST::FileNode> file = std::make_unique<AST::FileNode>();
    file->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    
    while(cursor.hasNext()) {
        if(Rules::isFunctionDefinition(cursor)) {
            auto function = parseFunctionDefinition(cursor);
            
            if(!function)
                return std::unexpected{function.error()};

            file->m_functions.push_back(std::move(function.value()));
            continue;
        }
        else if(Rules::isStructDefinition(cursor)) {
            auto type = parseStructDefinition(cursor);

            if(!type)
                return std::unexpected{type.error()};

            file->m_structs.push_back(std::move(type.value()));
            continue;
        }
        else if(Rules::isImport(cursor)) {
            auto import = parseImport(cursor);

            if(!import)
                return std::unexpected{import.error()};

            file->m_imports.push_back(std::move(import.value()));
            continue;
        }

        return unexpectedToken(cursor.get().value());
    }

    file->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    return file;
}