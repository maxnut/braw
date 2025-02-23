#include "lexer.hpp"

#include <spdlog/spdlog.h>

#include <fstream>

static std::unordered_map<std::string, Token::Type> s_tokenTypes = {
    {"int", Token::KEYWORD},
    {"char", Token::KEYWORD},
    {"double", Token::KEYWORD},
    {"float", Token::KEYWORD},
    {"void", Token::KEYWORD},
    {"false", Token::KEYWORD},
    {"true", Token::KEYWORD},
    {"bool", Token::KEYWORD},
    {"if", Token::KEYWORD},
    {"else", Token::KEYWORD},
    {"while", Token::KEYWORD},
    {"for", Token::KEYWORD},
    {"return", Token::KEYWORD},
    {"break", Token::KEYWORD},
    {"continue", Token::KEYWORD},
    {"struct", Token::KEYWORD},
    {"import", Token::KEYWORD},
    {"bind", Token::KEYWORD},
    {"nullptr", Token::KEYWORD},
    {";", Token::SEMICOLON},
    {"(", Token::LEFT_PAREN},
    {")", Token::RIGHT_PAREN},
    {"{", Token::LEFT_BRACE},
    {"}", Token::RIGHT_BRACE},
    {"[", Token::LEFT_BRACKET},
    {"]", Token::RIGHT_BRACKET},
    {"=", Token::ASSIGNMENT},
    {".", Token::OPERATOR},
    {"+", Token::OPERATOR},
    {"-", Token::OPERATOR},
    {"*", Token::OPERATOR},
    {"/", Token::OPERATOR},
    {"&", Token::OPERATOR},
    {"@", Token::OPERATOR},
    {"<", Token::OPERATOR},
    {">", Token::OPERATOR},
    {"<=", Token::OPERATOR},
    {">=", Token::OPERATOR},
    {"==", Token::OPERATOR},
    {"!=", Token::OPERATOR},
    {"&&", Token::OPERATOR},
    {"||", Token::OPERATOR},
    {"!", Token::OPERATOR},
    {"->", Token::OPERATOR},
    {"%", Token::OPERATOR},
    {"++", Token::OPERATOR},
    {"--", Token::OPERATOR},
    {",", Token::COMMA},
    {"\"", Token::QUOTE},
    {":", Token::COLON}
};
static std::unordered_map<char, char> s_escapeMap = {
    {'n', '\n'},
    {'t', '\t'},
};

Token parseNumber(Cursor<std::string::iterator>& cursor, int lineNumber) {
    std::string n = "";
    size_t index = cursor.getIndex() + 1;

    if(cursor.get().value() == '-')
        n += cursor.get().next().value();

    while(cursor.hasNext() && std::isdigit(cursor.get().value()) || cursor.get().value() == '.')
        n += cursor.get().next().value();

    if(n.find('.') != std::string::npos) {
        if(cursor.get().value() == 'f' || cursor.get().value() == 'F') {
            cursor.next();
            return Token(Token::FLOAT, n, lineNumber, index);
        }

        if(cursor.get().value() == 'l' || cursor.get().value() == 'L') {
            cursor.next();
            return Token(Token::LONG, n, lineNumber, index);
        }

        return Token(Token::DOUBLE, n, lineNumber, index);
    }

    return Token(Token::INTEGER, n, lineNumber, index);
}

Token parseAlphanumeric(Cursor<std::string::iterator>& cursor, int lineNumber) {
    std::string n = "";
    size_t index = cursor.getIndex() + 1;
    while(cursor.hasNext() && (std::isalnum(cursor.get().value()) || cursor.get().value() == '_'))
        n += cursor.get().next().value();
    return Token(Token::IDENTIFIER, n, lineNumber, index);
}

Token parseString(Cursor<std::string::iterator>& cursor, int lineNumber) {
    std::string ret = "";
    size_t index = cursor.getIndex() + 1;
    while(cursor.hasNext() && cursor.get().value() != '"') {

        if (cursor.get().value() == '\\') {
            char nextChar = cursor.next().get().prev().value();
            auto it = s_escapeMap.find(nextChar);
            if (it != s_escapeMap.end()) {
                ret += it->second;
                cursor.next(2);
                continue;
            }
        }

        ret += cursor.get().next().value();
    }
    return Token(Token::STRING, ret, lineNumber, index);
}


Token tryParseSingleToken(Cursor<std::string::iterator> cursor, int lineNumber) {
    Token token(Token::COUNT, lineNumber, cursor.getIndex() + 1);
    std::string val = "";

    while(cursor.hasNext() && !std::isspace(cursor.get().value())) {
        val += cursor.get().next().value();

        if(s_tokenTypes.contains(val)) {
            token.m_type = s_tokenTypes.at(val);
            token.m_value = val;
            continue;
        }
    }

    if(token.m_type == Token::COUNT)
        return {};
    
    return token;
}

std::optional<std::vector<Token>> Lexer::tokenize(std::filesystem::path path) {
    if(path.extension() != ".braw") {
        spdlog::error("guy {}", path.extension().string());
        return std::nullopt;
    }
    std::vector<Token> tokens;

    std::ifstream file(path);
    std::string line;
    int lineNumber = 1;

    while (std::getline(file, line)) {
        Cursor<std::string::iterator> cursor(line.begin(), line.end());

        while(cursor.hasNext()) {
            if(std::isspace(cursor.get().value())) {
                cursor.next();
                continue;
            }

            if(cursor.get().value() == '/' && cursor.next().get().prev().value() == '/') {
                cursor.next(2);
                while(cursor.hasNext() && cursor.get().value() != '\n')
                    cursor.next();
                continue;
            }

            if((cursor.get().value() == '-' && std::isdigit(cursor.peekNext())) || std::isdigit(cursor.get().value())) {
                tokens.push_back(parseNumber(cursor, lineNumber));
                continue;
            }

            if (Token val = tryParseSingleToken(cursor, lineNumber); val.m_type != Token::COUNT) {
                Token check = tokens.size() > 0 ? tokens.back() : Token();

                tokens.push_back(val);
                cursor.next(val.m_value.size());

                if(val.m_type != Token::QUOTE || check.m_type == Token::STRING)
                    continue;

                tokens.push_back(parseString(cursor, lineNumber));

                continue;
            }

            if(std::isalpha(cursor.get().value())) {
                tokens.push_back(parseAlphanumeric(cursor, lineNumber));
                continue;
            }

            spdlog::error("Unknown token at line {} column {}", lineNumber, cursor.getIndex() + 1);
            return std::nullopt;
        }

        lineNumber++;
    }

    if(tokens.size() == 0)
        return std::nullopt;

    return tokens;
}