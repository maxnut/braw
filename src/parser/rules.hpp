#pragma once

#include "../cursor.hpp"
#include "../token.hpp"

#include <vector>

using TokenCursor = Cursor<std::vector<Token>::iterator, Token>;

namespace Rules {
    inline bool isValidTypeName(TokenCursor& cursor) {
        if(cursor.get().value().m_type != Token::IDENTIFIER && cursor.get().value().m_type != Token::KEYWORD)
            return false;

        while(cursor.hasNext()) {
            if(cursor.next().get().value().m_value == "*")
                continue;
        }

        cursor.next();
        return true;
    }

    inline bool isFunctionDefinition(TokenCursor cursor) {
        if(!isValidTypeName(cursor))
            return false;

        if(cursor.get().value().m_type != Token::IDENTIFIER)
            return false;

        if(cursor.next().get().value().m_type != Token::LEFT_PAREN)
            return false;

        return true;
    }

    inline bool isAssignment(TokenCursor cursor) {
        while(cursor.hasNext() && cursor.get().value().m_type != Token::SEMICOLON) {
            if(cursor.get().next().value().m_value == "=")
                return true;
        }
        return false;
    }

    inline bool isVariableDeclaration(TokenCursor cursor) {
        if(!isValidTypeName(cursor))
            return false;

        if(cursor.get().value().m_type != Token::IDENTIFIER)
            return false;
        
        return true;
    }
}