#include "parser/parser.hpp"
#include "../cast.hpp"

std::unique_ptr<EvaluatableNode> Parser::parseCast(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_PAREN))
        return nullptr;

    std::optional<TypeInfo> typeOpt = parseTypename(file, cursor);
    if(!typeOpt) {
        m_message.unknownType(cursor.value().m_value);
        return nullptr;
    }
    TypeInfo type = typeOpt.value();

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return nullptr;

    std::unique_ptr<EvaluatableNode> base = parseOperand(file, cursor, ctx);
    if(!base)
        return nullptr;

    if(Rules::canDirectCast(base->m_type, type)) {
        base->m_type = type;
        return base;
    }

    if(!base->m_type.m_casts.contains(type.m_name)) {
        m_message.invalidCast(base->m_type.m_name, type.m_name);
        return nullptr;
    }

    std::unique_ptr<CastNode> castNode = std::make_unique<CastNode>();
    castNode->m_type = type;
    castNode->m_function = base->m_type.m_casts[type.m_name].m_function;
    castNode->m_base = std::move(base);

    return castNode;
}