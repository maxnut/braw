#include "parser/parser.hpp"
#include "../struct.hpp"

Result<std::shared_ptr<AST::StructNode>> Parser::parseStructDefinition(TokenCursor& cursor, ParserContext& ctx) {
    std::shared_ptr<AST::StructNode> structNode = std::make_shared<AST::StructNode>();
    structNode->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    
    if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
        return unexpectedTokenExpectedType(cursor.value(), Token::KEYWORD, ctx.m_path);

    if(!expectTokenValue(cursor.get().value(), "struct"))
        return unexpectedTokenExpectedValue(cursor.value(), "struct", ctx.m_path);

    if(!expectTokenType(cursor.next().get().value(), Token::IDENTIFIER))
        return unexpectedTokenExpectedType(cursor.value(), Token::IDENTIFIER, ctx.m_path);
    
    structNode->m_name = cursor.get().next().value().m_value;

    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_BRACE))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_BRACE, ctx.m_path);

    while(cursor.get().value().m_type != Token::RIGHT_BRACE) {
        auto optVar = parseVariableDeclaration(cursor, ctx, true);
        if(!optVar)
            return std::unexpected{optVar.error()};

        structNode->m_members.push_back(std::move(optVar.value()));
        cursor.next();
    }

    if(!expectTokenType(cursor.next().get().value(), Token::SEMICOLON))
        return unexpectedTokenExpectedType(cursor.value(), Token::SEMICOLON, ctx.m_path);
    
    structNode->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    cursor.tryNext();

    return structNode;
}