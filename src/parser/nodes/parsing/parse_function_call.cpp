#include "parser/parser.hpp"
#include "../function_call.hpp"
#include "../native_function_call.hpp"

std::unique_ptr<EvaluatableNode> Parser::parseFunctionCall(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    if(!expectTokenType(cursor.get().value(), Token::IDENTIFIER))
        return nullptr;

    std::string name = cursor.value().m_value;

    if(!expectTokenType(cursor.next().get().next().value(), Token::LEFT_PAREN))
        return nullptr;

    std::vector<std::unique_ptr<EvaluatableNode>> parameters;

    while(cursor.hasNext()) {
        parameters.push_back(parseExpression(file, cursor, ctx));

        if(!parameters.back())
            return nullptr;

        if(cursor.get().value().m_type == Token::RIGHT_PAREN)
            break;

        if(!expectTokenType(cursor.get().next().value(), Token::COMMA))
            return nullptr;
    }

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return nullptr;

    auto func = file->getFunction(name, parameters);
    if(!func) {
        m_message.unknownFunction(name, parameters);
        return nullptr;
    }

    if(func->isNative()) {
        std::unique_ptr<NativeFunctionCallNode> functionCall = std::make_unique<NativeFunctionCallNode>();
        functionCall->m_function = func;
        functionCall->m_parameters = std::move(parameters);
        functionCall->m_memoryType = ValueType::PRVALUE;
        functionCall->m_size = functionCall->m_function->m_signature.m_returnType.m_size;
        functionCall->m_type = functionCall->m_function->m_signature.m_returnType;
        return functionCall;
    }

    std::unique_ptr<FunctionCallNode> functionCall = std::make_unique<FunctionCallNode>();
    functionCall->m_function = func;
    functionCall->m_parameters = std::move(parameters);
    functionCall->m_memoryType = ValueType::PRVALUE;
    functionCall->m_size = functionCall->m_function->m_signature.m_returnType.m_size;
    functionCall->m_type = functionCall->m_function->m_signature.m_returnType;
    return functionCall;
}