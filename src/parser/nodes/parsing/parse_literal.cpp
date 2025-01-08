#include "parser/parser.hpp"
#include "../literal.hpp"

std::unique_ptr<EvaluatableNode> Parser::parseLiteral(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    std::unique_ptr<LiteralNode> literal = std::make_unique<LiteralNode>();

    Token tkn = cursor.get().value();

    if(tkn.m_type == Token::INTEGER) {
        literal->m_type = file->getTypeInfo("int").value();
        literal->m_value.m_data = new int;
        literal->m_value.from(std::stoi(cursor.value().m_value));
    }
    else if(tkn.m_type == Token::FLOAT) {
        literal->m_type = file->getTypeInfo("float").value();
        literal->m_value.m_data = new float;
        literal->m_value.from(std::stof(cursor.value().m_value));
    }
    else if(tkn.m_type == Token::DOUBLE) {
        literal->m_type = file->getTypeInfo("double").value();
        literal->m_value.m_data = new double;
        literal->m_value.from(std::stod(cursor.value().m_value));
    }
    else if(tkn.m_type == Token::KEYWORD) {
        if(tkn.m_value == "true") {
            literal->m_type = file->getTypeInfo("bool").value();
            literal->m_value.m_data = new bool;
            literal->m_value.from(true);
        }
        else if(tkn.m_value == "false") {
            literal->m_type = file->getTypeInfo("bool").value();
            literal->m_value.m_data = new bool;
            literal->m_value.from(false);
        }
        else
            return nullptr;
    }
    else
        return nullptr;

    literal->m_size = literal->m_type.m_size;
    literal->m_memoryType = ValueType::PRVALUE;

    cursor.next();

    return literal;
}