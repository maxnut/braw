#include "semantic_analyzer.hpp"
#include "parser/nodes/struct.hpp"

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::StructNode* node, BrawContext& ctx) {
    TypeInfo info{node->m_name};
    size_t size = 0;

    for(auto& member : node->m_members) {
        if(!ctx.getTypeInfo(member->m_type))
            return unknownType(node, member->m_type, ctx);

        info.m_members[member->m_name] = MemberInfo{member->m_type, size, member->m_scale};
        size += ctx.getTypeInfo(member->m_type)->m_size * member->m_scale;
    }

    info.m_size = size;
    ctx.m_typeTable[node->m_name] = info;

    return std::nullopt;
}