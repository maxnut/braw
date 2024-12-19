#pragma once

#include "../cursor.hpp"
#include "../token.hpp"

#include <vector>

namespace Rules {
    inline bool isValidTypeName(Cursor<std::vector<Token>::iterator, Token>& cursor) {
        if(cursor.get().value().m_type != Token::IDENTIFIER && cursor.get().value().m_type != Token::KEYWORD)
            return false;

        while(cursor.hasNext()) {
            if(cursor.next().get().value().m_value == "*")
                continue;
        }

        cursor.next();
        return true;
    }

    inline bool isFunctionDefinition(Cursor<std::vector<Token>::iterator, Token> cursor) {
        if(!isValidTypeName(cursor))
            return false;

        if(cursor.get().value().m_type != Token::IDENTIFIER)
            return false;

        if(cursor.next().get().value().m_type != Token::LEFT_PAREN)
            return false;

        return true;
    }
}