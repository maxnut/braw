#include "et_builder.hpp"
#include "parser/nodes/literal.hpp"
#include "execution-tree/nodes/literal.hpp"
#include "utils.hpp"

#include <cstdlib>
#include <cstring>
#include <memory>
#include <unordered_map>
#include <variant>

std::unique_ptr<EvaluatableNode> ETBuilder::buildLiteral(const AST::LiteralNode* node, BrawContext& context) {
    std::unique_ptr<LiteralNode> literal = std::make_unique<LiteralNode>();

    std::array<TypeInfo, 7> types = {
        context.getTypeInfo("int").value(), context.getTypeInfo("long").value(), context.getTypeInfo("float").value(), context.getTypeInfo("double").value(),
        context.getTypeInfo("bool").value(), Utils::makePointer(context.getTypeInfo("char").value()), Utils::makePointer(context.getTypeInfo("void").value())
    };
    literal->m_value = node->m_value;
    literal->m_type = types.at(node->m_value.index());
    literal->m_size = literal->m_type.m_size;
    
    return literal;
}