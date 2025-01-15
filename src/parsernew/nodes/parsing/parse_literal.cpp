#include "parsernew/parser.hpp"
#include "../literal.hpp"

Result<std::shared_ptr<AST::LiteralNode>> Parser::parseLiteral(TokenCursor& cursor) {
    std::shared_ptr<AST::LiteralNode> literal = std::make_shared<AST::LiteralNode>();
    Token tkn = cursor.get().value();

    if(tkn.m_type == Token::INTEGER) {
        long v = std::stol(cursor.value().m_value);
        literal->m_value = v > INT_MAX ? v : (int)v;
    }
    else if(tkn.m_type == Token::FLOAT) {
        literal->m_value = std::stof(cursor.value().m_value);
    }
    else if(tkn.m_type == Token::DOUBLE) {
        literal->m_value = std::stod(cursor.value().m_value);
    }
    else if(tkn.m_type == Token::KEYWORD) {
        if(tkn.m_value == "true") {
            literal->m_value = true;
        }
        else if(tkn.m_value == "false") {
            literal->m_value = false;
        }
        else if(tkn.m_value == "nullptr") {
            literal->m_value = nullptr;
        }
        else
            return std::unexpected{ParseError{
                "Invalid literal: " + tkn.m_value,
                tkn.m_line,
                tkn.m_column,
            }};
    }
    else if(tkn.m_type == Token::QUOTE) {
        literal->m_value = cursor.next().get().value().m_value;

        if(!expectTokenType(cursor.next().get().value(), Token::QUOTE))
            return unexpectedTokenExpectedType(cursor.value(), Token::QUOTE);
    }
    else
        return std::unexpected{ParseError{
                "Invalid literal: " + tkn.m_value,
                tkn.m_line,
                tkn.m_column,
            }};

    cursor.tryNext();

    return literal;
}