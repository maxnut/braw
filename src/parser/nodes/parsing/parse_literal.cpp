#include "parser/parser.hpp"
#include "../literal.hpp"

#include <cstring>

std::unique_ptr<EvaluatableNode> Parser::parseLiteral(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    std::unique_ptr<LiteralNode> literal = std::make_unique<LiteralNode>();

    Token tkn = cursor.get().value();

    if(tkn.m_type == Token::INTEGER) {
        literal->m_type = file->getTypeInfo("int").value();
        literal->m_value.m_data = malloc(sizeof(int));
        literal->m_value.from(std::stoi(cursor.value().m_value));
    }
    else if(tkn.m_type == Token::FLOAT) {
        literal->m_type = file->getTypeInfo("float").value();
        literal->m_value.m_data = malloc(sizeof(float));
        literal->m_value.from(std::stof(cursor.value().m_value));
    }
    else if(tkn.m_type == Token::DOUBLE) {
        literal->m_type = file->getTypeInfo("double").value();
        literal->m_value.m_data = malloc(sizeof(double));
        literal->m_value.from(std::stod(cursor.value().m_value));
    }
    else if(tkn.m_type == Token::KEYWORD) {
        if(tkn.m_value == "true") {
            literal->m_type = file->getTypeInfo("bool").value();
            literal->m_value.m_data = malloc(sizeof(bool));
            literal->m_value.from(true);
        }
        else if(tkn.m_value == "false") {
            literal->m_type = file->getTypeInfo("bool").value();
            literal->m_value.m_data = malloc(sizeof(bool));
            literal->m_value.from(false);
        }
        else if(tkn.m_value == "nullptr") {
            literal->m_type = makePointer(file->getTypeInfo("void").value());
            literal->m_value.m_data = malloc(sizeof(void*));
            literal->m_value.from(nullptr);
        }
        else
            return nullptr;
    }
    else if(tkn.m_type == Token::QUOTE) {
        std::string str = cursor.next().get().value().m_value;
        char* ptr = (char*)malloc(str.size() + 1);
        std::memcpy(ptr, str.data(), str.size() + 1);
        LiteralNode::s_strings.push_back(std::shared_ptr<char>(ptr));

        literal->m_type = makePointer(file->getTypeInfo("char").value());
        literal->m_value.m_data = malloc(sizeof(char*));
        literal->m_value.from(ptr);

        if(!expectTokenType(cursor.next().get().value(), Token::QUOTE))
            return nullptr;
    }
    else
        return nullptr;

    literal->m_size = literal->m_type.m_size;
    literal->m_memoryType = ValueType::PRVALUE;

    cursor.next();

    return literal;
}