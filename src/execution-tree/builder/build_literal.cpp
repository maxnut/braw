#include "et_builder.hpp"
#include "parser/nodes/literal.hpp"
#include "execution-tree/nodes/literal.hpp"
#include "utils.hpp"

#include <cstdlib>
#include <cstring>
#include <memory>
#include <unordered_map>
#include <variant>

std::unordered_map<const AST::LiteralNode*, Memory> s_literalCache;

std::unique_ptr<EvaluatableNode> ETBuilder::buildLiteral(const AST::LiteralNode* node, BrawContext& context) {
    std::unique_ptr<LiteralNode> literal = std::make_unique<LiteralNode>();

    if(s_literalCache.contains(node))
        literal->m_value = s_literalCache[node];
    else {
        Memory data;
        switch(node->m_value.index()) {
            case 0:
                data.m_data = malloc(sizeof(int));
                data.m_size = sizeof(int);
                data.from(std::get<int>(node->m_value));
                literal->m_type = context.getTypeInfo("int").value();
                break;
            case 1:
                data.m_data = malloc(sizeof(long));
                data.m_size = sizeof(long);
                data.from(std::get<long>(node->m_value));
                literal->m_type = context.getTypeInfo("long").value();
                break;
            case 2:
                data.m_data = malloc(sizeof(float));
                data.m_size = sizeof(float);
                data.from(std::get<float>(node->m_value));
                literal->m_type = context.getTypeInfo("float").value();
                break;
            case 3:
                data.m_data = malloc(sizeof(double));
                data.m_size = sizeof(double);
                data.from(std::get<double>(node->m_value));
                literal->m_type = context.getTypeInfo("double").value();
                break;
            case 4:
                data.m_data = malloc(sizeof(double));
                data.m_size = sizeof(double);
                data.from(std::get<bool>(node->m_value));
                literal->m_type = context.getTypeInfo("bool").value();
                break;
            case 5: {
                std::string literalStr = std::get<std::string>(node->m_value);
                std::shared_ptr<char> str = std::shared_ptr<char>((char*)malloc(literalStr.size() + 1));
                std::memcpy(str.get(), literalStr.c_str(), literalStr.size() + 1);
                data.from(str.get());
                literal->m_type = Utils::makePointer(context.getTypeInfo("char").value());
                LiteralNode::s_strings.push_back(str);
                break;
            }
            case 6:
                data.m_data = malloc(sizeof(void*));
                data.m_size = sizeof(void*);
                data.from(std::get<std::nullptr_t>(node->m_value));
                literal->m_type = Utils::makePointer(context.getTypeInfo("void").value());
                break;
        }
        literal->m_value = data;
        s_literalCache[node] = data;
    }

    literal->m_size = literal->m_value.m_size;
    
    return literal;
}