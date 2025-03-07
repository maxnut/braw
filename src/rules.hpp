#pragma once

#include "cursor.hpp"
#include "token.hpp"

#include <vector>
#include <unordered_map>

using TokenCursor = Cursor<std::vector<Token>::iterator>;

namespace Rules {
    enum InstructionType {
        FUNCTION_DEFINITION,
        ASSIGNMENT,
        RETURN,
        IF,
        WHILE,
        VARIABLE_DECLARATION,
        FUNCTION_CALL,
        VARIABLE_ACCESS,
        COUNT
    };

    inline std::unordered_map<std::string, int> s_operatorPrecedence = {
        {"+", 0},
        {"-", 0},
        {"*", 1},
        {"/", 1},
    };

    inline bool isValidTypeName(TokenCursor& cursor) {
        if((cursor.get().value().m_type != Token::IDENTIFIER && cursor.get().value().m_type != Token::KEYWORD) || cursor.value().m_value == "return")
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
        while(cursor.hasNext() && cursor.get().value().m_type != Token::SEMICOLON && cursor.get().value().m_type != Token::LEFT_BRACE && cursor.get().value().m_type != Token::COLON) {
            if(cursor.get().next().value().m_value == "=")
                return true;
        }
        return false;
    }

    inline bool isReturn(TokenCursor cursor) {
        return cursor.get().value().m_type == Token::KEYWORD && cursor.get().value().m_value == "return";
    }

    inline bool isIf(TokenCursor cursor) {
        return cursor.get().value().m_type == Token::KEYWORD && cursor.get().value().m_value == "if";
    }

    inline bool isWhile(TokenCursor cursor) {
        return cursor.get().value().m_type == Token::KEYWORD && cursor.get().value().m_value == "while";
    }

    inline bool isVariableDeclaration(TokenCursor cursor) {
        if(cursor.get().value().m_type != Token::KEYWORD || cursor.get().value().m_value != "let")
            return false;
        cursor.next();

        if(cursor.get().value().m_type != Token::IDENTIFIER)
            return false;
        
        if(cursor.next().get().next().value().m_type != Token::COLON)
            return false;

        if(!isValidTypeName(cursor))
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

    inline bool isStructDefinition(TokenCursor cursor) {
        if(cursor.get().value().m_value != "struct" || cursor.next().get().value().m_type != Token::IDENTIFIER)
            return false;

        return true;
    }

    inline bool isImport(TokenCursor cursor) {
        if(cursor.get().value().m_value != "import")
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
        if(cursor.get().value().m_type != Token::INTEGER && cursor.get().value().m_type != Token::FLOAT
            && cursor.get().value().m_type != Token::DOUBLE && !isString(cursor)
            && cursor.get().value().m_value != "true" && cursor.get().value().m_value != "false"
            && cursor.get().value().m_value != "nullptr")
            return false;

        return true;
    }

    inline bool isPtr(const std::string& type) {
        return type.find("*") != std::string::npos;
    }

    inline bool isCast(TokenCursor cursor) {
        if(cursor.get().next().value().m_type != Token::LEFT_PAREN || !isValidTypeName(cursor) || cursor.get().value().m_type != Token::RIGHT_PAREN)
            return false;

        return true;
    }

    inline bool canDirectCast(const std::string& a, const std::string& b) {
        if(Rules::isPtr(a) && Rules::isPtr(b))
            return true;

        return false;
    }

    inline InstructionType getInstructionType(TokenCursor cursor) {
        if(isFunctionDefinition(cursor))
            return InstructionType::FUNCTION_DEFINITION;
        else if(isAssignment(cursor))
            return InstructionType::ASSIGNMENT;
        else if(isReturn(cursor))
            return InstructionType::RETURN;
        else if(isIf(cursor))
            return InstructionType::IF;
        else if(isWhile(cursor))
            return InstructionType::WHILE;
        else if(isVariableDeclaration(cursor))
            return InstructionType::VARIABLE_DECLARATION;
        else if(isFunctionCall(cursor))
            return InstructionType::FUNCTION_CALL;
        else if(isVariableAccess(cursor))
            return InstructionType::VARIABLE_ACCESS;
        else
            return COUNT;
    }
}