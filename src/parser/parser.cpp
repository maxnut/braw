#include "parser.hpp"
#include "../lexer/lexer.hpp"

#include <spdlog/spdlog.h>

std::shared_ptr<FileNode> Parser::parse() {
    auto tokens = Lexer(m_file).tokenize();
    if(!tokens)
        return nullptr;
    return parseFile(tokens.value());
}

bool Parser::expectTokenType(const Token& token, Token::Type type) {
    if(token.m_type != type)
        m_message.expectedTokenType(Token::typeString(token.m_type), Token::typeString(type));

    return token.m_type == type;
}

bool Parser::expectTokenValue(const Token& token, const std::string& value) {
    if(token.m_value != value)
        m_message.unexpectedToken(token);

    return token.m_value == value;
}

std::optional<ScopeInfo> ParserFunctionContext::get(const std::string& name) const {
    for(auto it = m_scopeTables.rbegin(); it != m_scopeTables.rend(); it++) {
        auto it2 = it->find(name);
        if(it2 != it->end()) {
            return it2->second;
        }
    }
    return std::nullopt;
}

bool ParserFunctionContext::isDefined(const std::string& name) const {
    return get(name).has_value();
}

void ParserFunctionContext::changeStackSize(int amt) {
    m_currentStackSize += amt;
    if(m_currentStackSize > m_maxStackSize)
        m_maxStackSize = m_currentStackSize;
}

void ParserFunctionContext::setStackSize(size_t amt) {
    m_currentStackSize = amt;
    if(m_currentStackSize > m_maxStackSize)
        m_maxStackSize = m_currentStackSize;
}

TypeInfo Parser::makePointer(const TypeInfo& base) {
    return TypeInfo{base.m_name + "*", 8};
}

std::optional<TypeInfo> Parser::getRawType(const TypeInfo& pointer, std::shared_ptr<FileNode> file) {
    std::string raw = pointer.m_name;
    if(raw.find("*") != std::string::npos)
        raw = raw.substr(0, raw.size() - 1);

    if(Rules::isPtr(raw))
        return TypeInfo{raw, 8};

    return file->getTypeInfo(raw);
}

uint32_t Parser::getPointerDepth(const TypeInfo& pointer) {
    uint32_t depth = 0;
    std::string name = pointer.m_name;
    while(pointer.m_name.find("*") != std::string::npos) {
        depth++;
        name = name.substr(0, name.size() - 1);
    }

    return depth;
}