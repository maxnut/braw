#include "parser/parser.hpp"
#include "../struct.hpp"

Result<std::shared_ptr<AST::StructNode>> Parser::parseStructDefinition(TokenCursor& cursor, const std::filesystem::path& path, std::shared_ptr<AST::FileNode> file) {
    std::shared_ptr<AST::StructNode> structNode = std::make_shared<AST::StructNode>();
    structNode->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    
    if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
        return unexpectedTokenExpectedType(cursor.value(), Token::KEYWORD, path);

    if(!expectTokenValue(cursor.get().value(), "struct"))
        return unexpectedTokenExpectedValue(cursor.value(), "struct", path);

    if(!expectTokenType(cursor.next().get().value(), Token::IDENTIFIER))
        return unexpectedTokenExpectedType(cursor.value(), Token::IDENTIFIER, path);
    
    structNode->m_name = cursor.get().next().value().m_value;

    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_BRACE))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_BRACE, path);

    while(cursor.get().value().m_type != Token::RIGHT_BRACE) {
        auto optVar = parseVariableDeclaration(cursor, path, file, true);
        if(!optVar)
            return std::unexpected{optVar.error()};

        structNode->m_members.push_back(std::move(optVar.value()));
        cursor.next();
    }

    if(!expectTokenType(cursor.next().get().value(), Token::SEMICOLON))
        return unexpectedTokenExpectedType(cursor.value(), Token::SEMICOLON, path);
    
    structNode->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    cursor.tryNext();

    return structNode;
}