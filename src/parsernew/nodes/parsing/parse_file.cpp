#include "parsernew/parser.hpp"
#include "../file.hpp"

Result<std::unique_ptr<AST::FileNode>> Parser::parseFile(TokenCursor& cursor) {
    std::unique_ptr<AST::FileNode> file = std::make_unique<AST::FileNode>();
    
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
        else if(Rules::isBind(cursor)) {
            auto bind = parseBind(cursor);

            if(!bind)
                return std::unexpected{bind.error()};

            file->m_binds.push_back(std::move(bind.value()));
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

    return file;
}