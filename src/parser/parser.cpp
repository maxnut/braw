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

std::string Parser::getRawType(const std::string& type) {
    if(type.find("*") != std::string::npos)
        return type.substr(0, type.size() - 1);
    return type;
}