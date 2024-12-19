#include "../../parser.hpp"
#include "../../rules.hpp"

std::unique_ptr<FileNode> Parser::parseFile(std::vector<Token>& tokens) {
    std::unique_ptr<FileNode> file = std::make_unique<FileNode>();

    Cursor<std::vector<Token>::iterator, Token> cursor(tokens.begin(), tokens.end());

    while(cursor.hasNext()) {
        if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
            return nullptr;
        
        if(Rules::isFunctionDefinition(cursor)) {

        }
    }

    return file;
}