#include "parsernew/parser.hpp"
#include "../struct.hpp"

Result<std::shared_ptr<AST::StructNode>> Parser::parseStructDefinition(TokenCursor& cursor) {
    if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
        return unexpectedTokenExpectedType(cursor.value(), Token::KEYWORD);

    if(!expectTokenValue(cursor.get().value(), "struct"))
        return unexpectedTokenExpectedValue(cursor.value(), "struct");

    if(!expectTokenType(cursor.next().get().value(), Token::IDENTIFIER))
        return unexpectedTokenExpectedType(cursor.value(), Token::IDENTIFIER);
        
    std::shared_ptr<AST::StructNode> structNode = std::make_shared<AST::StructNode>();
    structNode->m_name = cursor.get().next().value().m_value;

    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_BRACE))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_BRACE);

    while(cursor.get().value().m_type != Token::RIGHT_BRACE) {
        std::shared_ptr<AST::VariableDeclarationNode> varDecl = std::make_shared<AST::VariableDeclarationNode>();
        varDecl->m_type = cursor.get().next().value().m_value;

        if(!expectTokenType(cursor.get().value(), Token::IDENTIFIER))
            return unexpectedTokenExpectedType(cursor.value(), Token::IDENTIFIER);

        varDecl->m_name = cursor.get().next().value().m_value;

        if(!expectTokenType(cursor.get().value(), Token::SEMICOLON))
            return unexpectedTokenExpectedType(cursor.value(), Token::SEMICOLON);
        
        structNode->m_members.push_back(std::move(varDecl));
        cursor.next();
    }

    if(!expectTokenType(cursor.next().get().value(), Token::SEMICOLON))
        return unexpectedTokenExpectedType(cursor.value(), Token::SEMICOLON);

    cursor.tryNext();

    return structNode;
}