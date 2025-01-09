#include "parser/parser.hpp"

std::shared_ptr<FileNode> Parser::parseFile(std::vector<Token>& tokens) {
    std::shared_ptr<FileNode> file = std::make_shared<FileNode>();

    TokenCursor cursor(tokens.begin(), tokens.end() - 1);

    m_message = Message(m_file, &cursor);

    while(cursor.hasNext()) {
        if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
            return nullptr;
        
        if(Rules::isFunctionDefinition(cursor)) {
            std::shared_ptr<FunctionDefinitionNode> node = parseFunctionDefinition(file, cursor);
            if(!node)
                return nullptr;

            if(!file->registerFunction(node, node->m_name))
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

        m_message.unexpectedToken(cursor.get().value());
        return nullptr;
    }

    return file;
}