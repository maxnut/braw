#include "parser/parser.hpp"
#include "../function_call.hpp"

std::unique_ptr<EvaluatableNode> Parser::parseFunctionCall(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    if(!expectTokenType(cursor.get().value(), Token::IDENTIFIER))
        return nullptr;

    std::string name = cursor.value().m_value;

    if(!expectTokenType(cursor.next().get().next().value(), Token::LEFT_PAREN))
        return nullptr;

    std::unique_ptr<FunctionCallNode> functionCall = std::make_unique<FunctionCallNode>();

    while(cursor.hasNext()) {
        functionCall->m_parameters.push_back(parseExpression(file, cursor, ctx));

        if(cursor.get().value().m_type == Token::RIGHT_PAREN)
            break;

        if(!expectTokenType(cursor.get().value(), Token::COMMA))
            return nullptr;
    }

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return nullptr;

    functionCall->m_function = file->getFunction(name, functionCall->m_parameters);
    if(!functionCall->m_function) {
        m_message.unknownFunction(name, functionCall->m_parameters);
        return nullptr;
    }

    functionCall->m_memoryType = ValueType::PRVALUE;
    functionCall->m_size = functionCall->m_function->m_returnType.m_size;
    functionCall->m_type = functionCall->m_function->m_returnType;
    
    return functionCall;
}