#include "parser/parser.hpp"

std::shared_ptr<FileNode> Parser::parseFile(std::vector<Token>& tokens) {
    std::shared_ptr<FileNode> file = std::make_shared<FileNode>();

    TokenCursor cursor(tokens.begin(), tokens.end() - 1);

    m_message = Message(m_file, &cursor);

    while(cursor.hasNext()) {
        if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
            return nullptr;
        
        if(Rules::isFunctionDefinition(cursor)) {

        }
    }

    return file;
}