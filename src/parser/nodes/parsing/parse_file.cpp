#include "parser/parser.hpp"
#include "lexer/lexer.hpp"

std::shared_ptr<FileNode> Parser::parseFile(std::filesystem::path filepath, std::vector<Token>& tokens) {
    m_file = filepath;
    std::shared_ptr<FileNode> file = std::make_shared<FileNode>();

    TokenCursor cursor(tokens.begin(), tokens.end() - 1);

    m_message = Message(m_file, &cursor);

    while(cursor.hasNext()) {
        if(Rules::isFunctionDefinition(cursor)) {
            std::shared_ptr<FunctionDefinitionNode> node = parseFunctionDefinition(file, cursor);
            if(!node)
                return nullptr;

            if(!file->registerFunction(node, node->m_signature.m_name))
                return nullptr;
            
            continue;
        }
        else if(Rules::isStructDefinition(cursor)) {
            std::optional<TypeInfo> type = parseStructDefinition(file, cursor);
            if(!type)
                return nullptr;

            if(!file->registerType(type.value()))
                return nullptr;
            
            continue;
        }
        else if(Rules::isBind(cursor)) {
            auto funcs = parseBind(file, cursor);

            for(auto func : funcs) {
                if(!func) {
                    return nullptr;
                }

                if(!file->registerFunction(func, func->m_signature.m_name))
                    return nullptr;
            }

            continue;
        }
        else if(Rules::isImport(cursor)) {
            if(!expectTokenType(cursor.next().get().value(), Token::QUOTE) || !expectTokenType(cursor.next().get().value(), Token::STRING))
                return nullptr;

            std::filesystem::path path = m_file.root_directory() / cursor.get().value().m_value;

            if(!expectTokenType(cursor.next().get().value(), Token::QUOTE))
                return nullptr;

            if(FileNode::s_fileCache.contains(path)) {
                file->m_imports.push_back(FileNode::s_fileCache.at(path));
            }
            else {
                auto newTokens = Lexer::tokenize(path);
                if(!newTokens)
                    return nullptr;

                std::shared_ptr<FileNode> newFile = Parser().parseFile(path, newTokens.value());

                if(!newFile)
                    return nullptr;

                file->m_imports.push_back(newFile);
            }

            cursor.tryNext();
            continue;
        }

        m_message.unexpectedToken(cursor.get().value());
        return nullptr;
    }

    return file;
}