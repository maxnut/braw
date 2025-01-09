#include "parser/parser.hpp"

std::optional<FunctionSignature> Parser::parseFunctionSignature(std::shared_ptr<FileNode> file, TokenCursor& cursor) {
    FunctionSignature sig;
    
    std::optional<TypeInfo> returnType = file->getTypeInfo(cursor.get().value().m_value);
    if(!returnType) {
        m_message.unknownType(cursor.value().m_value);
        return std::nullopt;
    }

    sig.m_returnType = returnType.value();
    sig.m_name = cursor.next().get().value().m_value;

    if(!expectTokenType(cursor.next().get().next().value(), Token::LEFT_PAREN))
        return std::nullopt;

    while(cursor.hasNext() && cursor.get().value().m_type != Token::RIGHT_PAREN) {
        std::optional<TypeInfo> typeOpt = file->getTypeInfo(cursor.get().value().m_value);
        if(!typeOpt) {
            m_message.unknownType(cursor.value().m_value);
            return std::nullopt;
        }

        TypeInfo type = typeOpt.value();

        while(cursor.hasNext() && cursor.peekNext().m_value == "*") {
            type = makePointer(type);
            cursor.next();
        }

        cursor.next();
        
        sig.m_parameterNames.push_back(cursor.value().m_value);
        sig.m_parameters.push_back(type);

        if(cursor.next().get().value().m_type == Token::COMMA)
            cursor.next();
        else if(cursor.value().m_type != Token::RIGHT_PAREN) {
            m_message.unexpectedToken(cursor.value());
            return std::nullopt;
        }
    }

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return std::nullopt;

    return sig;
}

std::shared_ptr<FunctionDefinitionNode> Parser::parseFunctionDefinition(std::shared_ptr<FileNode> file, TokenCursor& cursor) {
    std::shared_ptr<FunctionDefinitionNode> node = std::make_shared<FunctionDefinitionNode>();
    
    auto signatureOpt = parseFunctionSignature(file, cursor);
    if(!signatureOpt)
        return nullptr;

    node->m_signature = signatureOpt.value();

    ParserFunctionContext ctx;
    std::unordered_map<std::string, ScopeInfo> scopeTable = std::unordered_map<std::string, ScopeInfo>();

    for(int i = 0; i < node->m_signature.m_parameterNames.size(); i++) {
        scopeTable[node->m_signature.m_parameterNames[i]] = ScopeInfo(node->m_signature.m_parameters[i], ctx.m_currentStackSize, 0);
        ctx.changeStackSize(node->m_signature.m_parameters[i].m_size);
    }

    ctx.m_scopeTables.push_front(scopeTable);
    node->m_scope = parseScope(file, cursor, ctx);
    ctx.m_scopeTables.pop_front();

    if(!node->m_scope)
        return nullptr;

    return node;
}
