#include "parser/parser.hpp"
#include "../cast.hpp"

std::unique_ptr<EvaluatableNode> Parser::parseCast(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_PAREN))
        return nullptr;

    std::optional<TypeInfo> typeOpt = file->getTypeInfo(cursor.get().value().m_value);
    if(!typeOpt) {
        m_message.unknownType(cursor.value().m_value);
        return nullptr;
    }

    TypeInfo type = typeOpt.value();

    while(cursor.hasNext() && cursor.peekNext().m_value == "*") {
        type = makePointer(type);
        cursor.next();
    }

    cursor.next();

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return nullptr;

    std::unique_ptr<EvaluatableNode> base = parseExpression(file, cursor, ctx, 0);
    if(!base)
        return nullptr;

    if(Rules::canDirectCast(base->m_type, type)) {
        base->m_type = type;
        return base;
    }

    std::unique_ptr<CastNode> castNode = std::make_unique<CastNode>();
    castNode->m_type = type;
    castNode->m_base = std::move(base);
    //get cast func from map

    return castNode;
}