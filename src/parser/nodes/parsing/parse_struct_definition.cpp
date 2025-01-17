#include "parser/parser.hpp"
#include "../struct.hpp"

Result<std::unique_ptr<AST::StructNode>> Parser::parseStructDefinition(TokenCursor& cursor) {
    std::unique_ptr<AST::StructNode> structNode = std::make_unique<AST::StructNode>();
    structNode->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    
    if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
        return unexpectedTokenExpectedType(cursor.value(), Token::KEYWORD);

    if(!expectTokenValue(cursor.get().value(), "struct"))
        return unexpectedTokenExpectedValue(cursor.value(), "struct");

    if(!expectTokenType(cursor.next().get().value(), Token::IDENTIFIER))
        return unexpectedTokenExpectedType(cursor.value(), Token::IDENTIFIER);
    
    structNode->m_name = cursor.get().next().value().m_value;

    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_BRACE))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_BRACE);

    while(cursor.get().value().m_type != Token::RIGHT_BRACE) {
        std::unique_ptr<AST::VariableDeclarationNode> varDecl = std::make_unique<AST::VariableDeclarationNode>();
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
    
    structNode->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    cursor.tryNext();

    return structNode;
}