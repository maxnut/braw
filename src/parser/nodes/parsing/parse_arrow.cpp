#include "parser/parser.hpp"
#include "../arrow.hpp"

std::unique_ptr<EvaluatableNode> Parser::parseArrow(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx, std::unique_ptr<EvaluatableNode> left) {
    if(!expectTokenType(cursor.get().next().value(), Token::OPERATOR))
        return nullptr;

    if(left->m_memoryType != ValueType::LVALUE) {
        m_message.unexpectedValueCategories(left->m_memoryType, {ValueType::LVALUE});
        return nullptr;
    }

    if(!Rules::isPtr(left->m_type.m_name)) {
        m_message.mismatchedTypes("pointer", left->m_type.m_name);
        return nullptr;
    }

    auto rawTypeOpt = getRawType(left->m_type, file);
    if(!rawTypeOpt) {
        m_message.unknownType(left->m_type.m_name);
        return nullptr;
    }
    TypeInfo rawType = rawTypeOpt.value();

    std::string name = cursor.get().next().value().m_value;

    if(!rawType.m_members.contains(name)) {
        m_message.unknownMember(rawType.m_name, name);
        return nullptr;
    }

    MemberInfo info = rawType.m_members[name];
    TypeInfo typeInfo = file->getTypeInfo(info.m_type).value();

    std::unique_ptr<ArrowNode> dot = std::make_unique<ArrowNode>();
    dot->m_memoryType = ValueType::LVALUE;
    dot->m_offset = info.m_offset;
    dot->m_type = typeInfo;
    dot->m_size = typeInfo.m_size * (info.m_arraySize == 0 ? 1 : info.m_arraySize);
    dot->m_base = std::move(left);

    return dot;
}