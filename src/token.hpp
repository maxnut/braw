#pragma once

#include <string>
#include <vector>

class Token {
public:
    enum Type {
        LEFT_PAREN,
        RIGHT_PAREN,
        LEFT_BRACE,
        RIGHT_BRACE,
        LEFT_BRACKET,
        RIGHT_BRACKET,
        INTEGER,
        LONG,
        FLOAT,
        DOUBLE,
        KEYWORD,
        IDENTIFIER,
        SEMICOLON,
        ASSIGNMENT,
        OPERATOR,
        COMMA,
        QUOTE,
        STRING,
        COUNT
    };

    Token(Type type, std::string value, int line, int column) : m_type(type), m_value(value), m_line(line), m_column(column) {}
    Token(Type type, int line, int column) : m_type(type), m_line(line), m_column(column) {}
    Token() {}

    static std::string typeString(Type type) {
        switch(type) {
            case LEFT_PAREN: return "LEFT_PAREN";
            case RIGHT_PAREN: return "RIGHT_PAREN";
            case LEFT_BRACE: return "LEFT_BRACE";
            case RIGHT_BRACE: return "RIGHT_BRACE";
            case LEFT_BRACKET: return "LEFT_BRACKET";
            case RIGHT_BRACKET: return "RIGHT_BRACKET";
            case INTEGER: return "INTEGER";
            case LONG: return "LONG";
            case FLOAT: return "FLOAT";
            case DOUBLE: return "DOUBLE";
            case KEYWORD: return "KEYWORD";
            case IDENTIFIER: return "IDENTIFIER";
            case SEMICOLON: return "SEMICOLON";
            case ASSIGNMENT: return "ASSIGNMENT";
            case OPERATOR: return "OPERATOR";
            case COMMA: return "COMMA";
            case QUOTE: return "QUOTE";
            case STRING: return "STRING";
            case COUNT: return "UNKNOWN";
        }
        return "UNKNOWN";
    }

    static std::string typeString(const std::vector<Type>& types) {
        if(types.size() == 1) return typeString(types[0]);

        std::string res = "";

        for(auto type : types)
            res += typeString(type) + " | ";

        return res.substr(0, res.size() - 3);
    }

public:
    Type m_type = COUNT;
    std::string m_value = "";
    int m_line = 0;
    int m_column = 0;
};