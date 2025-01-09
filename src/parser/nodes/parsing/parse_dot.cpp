#include "parser/parser.hpp"
#include "../dot.hpp"

std::unique_ptr<EvaluatableNode> Parser::parseDot(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx, std::unique_ptr<EvaluatableNode> left) {
    if(!expectTokenType(cursor.get().next().value(), Token::OPERATOR))
        return nullptr;

    if(left->m_memoryType != ValueType::LVALUE) {
        m_message.unexpectedValueCategories(left->m_memoryType, {ValueType::LVALUE});
        return nullptr;
    }

    std::string name = cursor.get().next().value().m_value;

    if(!left->m_type.m_members.contains(name)) {
        m_message.unknownMember(left->m_type.m_name, name);
        return nullptr;
    }

    MemberInfo info = left->m_type.m_members[name];
    TypeInfo typeInfo = file->getTypeInfo(info.m_type).value();

    std::unique_ptr<DotNode> dot = std::make_unique<DotNode>();
    dot->m_memoryType = ValueType::LVALUE;
    dot->m_offset = info.m_offset;
    dot->m_type = typeInfo;
    dot->m_size = typeInfo.m_size * (info.m_arraySize == 0 ? 1 : info.m_arraySize);
    dot->m_base = std::move(left);

    return dot;
}