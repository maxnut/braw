#include "parser/parser.hpp"

std::optional<TypeInfo> Parser::parseStructDefinition(std::shared_ptr<FileNode> file, TokenCursor& cursor) {
    if(!expectTokenType(cursor.get().value(), Token::KEYWORD) || !expectTokenValue(cursor.get().value(), "struct"))
        return std::nullopt;

    if(!expectTokenType(cursor.next().get().value(), Token::IDENTIFIER))
        return std::nullopt;

    std::string structName = cursor.get().next().value().m_value;

    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_BRACE))
        return std::nullopt;

    TypeInfo info;
    info.m_name = structName;

    while(cursor.get().value().m_type != Token::RIGHT_BRACE) {
        std::string type = cursor.get().next().value().m_value;
        auto typeInfo = file->getTypeInfo(type);

        if(!typeInfo)
            return std::nullopt;

        if(!expectTokenType(cursor.get().value(), Token::IDENTIFIER))
            return std::nullopt;

        std::string name = cursor.get().next().value().m_value;
        size_t arraySize = 0;

        if(cursor.get().value().m_type == Token::LEFT_BRACKET) {
            if(!expectTokenType(cursor.next().get().value(), Token::INTEGER))
                return std::nullopt;

            arraySize = std::stoul(cursor.get().value().m_value);

            if(!expectTokenType(cursor.next().get().value(), Token::RIGHT_BRACKET))
                return std::nullopt;

            cursor.next();
        }

        if(!expectTokenType(cursor.get().value(), Token::SEMICOLON))
            return std::nullopt;

        info.m_members[name] = {type, info.m_size, arraySize};
        info.m_size += typeInfo->m_size;
        cursor.next();
    }

    if(!expectTokenType(cursor.next().get().value(), Token::SEMICOLON))
        return std::nullopt;

    cursor.next();

    return info;
}