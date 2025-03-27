#include "parser/identifier.hpp"
#include "parser/parser.hpp"
#include "../file.hpp"
#include "rules.hpp"
#include <cmath>
#include <memory>
#include <unordered_map>

Result<std::shared_ptr<AST::FileNode>> Parser::parseFile(TokenCursor& cursor, std::filesystem::path path) {
    static std::unordered_map<std::filesystem::path, std::shared_ptr<AST::FileNode>> importCache;
    if(importCache.contains(path))
        return importCache[path];
    
    std::shared_ptr<AST::FileNode> file = std::make_shared<AST::FileNode>();
    file->m_path = path;
    file->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};

    ParserContext ctx{path, file};
    
    while(cursor.hasNext()) {
        if(Rules::isFunctionDefinition(cursor)) {
            auto function = parseFunctionDefinition(cursor, ctx);
            
            if(!function)
                return std::unexpected{function.error()};

            file->m_functions.push_back(std::move(function.value()));
            continue;
        }
        else if(Rules::isStructDefinition(cursor)) {
            auto type = parseStructDefinition(cursor, ctx);

            if(!type)
                return std::unexpected{type.error()};

            file->m_structs.push_back(std::move(type.value()));
            continue;
        }
        else if(Rules::isImport(cursor)) {
            auto import = parseImport(cursor, ctx);

            if(!import)
                return std::unexpected{import.error()};

            file->m_imports.push_back(import.value());
            continue;
        }
        else if(Rules::isDefine(cursor)) {
            auto macro = parseMacro(cursor, ctx);

            if(!macro)
                return std::unexpected{macro.error()};

            file->m_macros.insert({macro.value()->m_name, macro.value()});
            continue;
        }
        else if(Rules::isMacroCall(cursor)) {
            auto call = parseMacroCall(cursor, ctx);

            if(!call)
                return std::unexpected{call.error()};

            if(!expectTokenType(cursor.get().next().value(), Token::SEMICOLON))
                return unexpectedTokenExpectedType(cursor.value(), Token::SEMICOLON, ctx.m_path);

            file->m_macroCalls.push_back(call.value());
            continue;
        }

        return unexpectedToken(cursor.get().value(), ctx.m_path);
    }

    file->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    importCache[path] = file;
    return file;
}