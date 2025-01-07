#pragma once

#include "../cursor.hpp"
#include "../token.hpp"

#include <vector>
#include <unordered_map>

using TokenCursor = Cursor<std::vector<Token>::iterator, Token>;

namespace Rules {
    inline std::unordered_map<std::string, int> s_operatorPrecedence = {
        {"+", 0},
        {"-", 0},
        {"*", 2},
        {"/", 1},
    };

    inline bool isValidTypeName(TokenCursor& cursor) {
        if(cursor.get().value().m_type != Token::IDENTIFIER && cursor.get().value().m_type != Token::KEYWORD)
            return false;

        while(cursor.hasNext()) {
            if(cursor.next().get().value().m_value == "*")
                continue;

            break;
        }

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

    inline bool isFunctionCall(TokenCursor cursor) {
        if(cursor.get().value().m_type != Token::IDENTIFIER)
            return false;

        if(cursor.next().get().value().m_type != Token::LEFT_PAREN)
            return false;

        return true;
    }

    inline bool isVariableAccess(TokenCursor cursor) {
        if(cursor.get().value().m_type != Token::IDENTIFIER)
            return false;

        return true;
    }

    inline bool isString(TokenCursor cursor) {
        if(cursor.get().value().m_type != Token::QUOTE)
            return false;

        if(cursor.next().get().value().m_type != Token::STRING)
            return false;

        if(cursor.next().get().value().m_type != Token::QUOTE)
            return false;

        return true;
    }

    inline bool isLiteral(TokenCursor cursor) {
        if(cursor.get().value().m_type != Token::INTEGER && cursor.get().value().m_type != Token::FLOAT && cursor.get().value().m_type != Token::DOUBLE && !isString(cursor))
            return false;

        return true;
    }

    inline bool isPtr(const std::string& type) {
        return type.find("*") != std::string::npos;
    }
}