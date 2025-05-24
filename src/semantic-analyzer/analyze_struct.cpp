#include "semantic_analyzer.hpp"
#include "parser/nodes/struct.hpp"
#include "utils.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::StructNode* node, BrawContext& ctx) {
    TypeInfo info{Utils::getIdentifier(node->m_name)};
    size_t size = 0;

    for(auto& member : node->m_members) {
        if(!ctx.getTypeInfo(Utils::getIdentifier(member->m_type)))
            return unknownType(node, Utils::getIdentifier(member->m_type), ctx);

        info.m_members[Utils::getIdentifier(member->m_name)] = MemberInfo{Utils::getIdentifier(member->m_type), size, member->m_scale};
        size += ctx.getTypeInfo(Utils::getIdentifier(member->m_type))->m_size * member->m_scale;
    }

    info.m_size = size;
    ctx.m_typeTable[Utils::getIdentifier(node->m_name)] = info;

    return std::nullopt;
}