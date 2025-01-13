#include "parser/parser.hpp"
#include "parser/binder/binder.hpp"

std::vector<std::shared_ptr<NativeFunctionNode>> Parser::parseBind(std::shared_ptr<FileNode> file, TokenCursor& cursor) {
    if(!expectTokenType(cursor.get().next().value(), Token::KEYWORD)
        || !expectTokenType(cursor.get().next().value(), Token::LEFT_PAREN)
        || !expectTokenType(cursor.get().next().value(), Token::QUOTE)
        || !expectTokenType(cursor.get().value(), Token::STRING))
        return {nullptr};

    std::string library = cursor.value().m_value;

    if(!expectTokenType(cursor.next().get().value(), Token::QUOTE)
        || !expectTokenType(cursor.next().get().value(), Token::RIGHT_PAREN)
        || !expectTokenType(cursor.next().get().next().value(), Token::LEFT_BRACE))
        return {nullptr};

    std::vector<std::shared_ptr<NativeFunctionNode>> funcs;

    while(cursor.hasNext() && cursor.get().value().m_type != Token::RIGHT_BRACE) {
        if(!expectTokenType(cursor.get().next().value(), Token::QUOTE) || !expectTokenType(cursor.get().value(), Token::STRING))
            return {nullptr};

        std::string symbol = cursor.get().next().value().m_value;
        
        if(!expectTokenType(cursor.get().next().value(), Token::QUOTE))
            return {nullptr};

        auto signatureOpt = parseFunctionSignature(file, cursor);
        if(!signatureOpt) {
            m_message.bindFail(library);
            return {nullptr};
        }

        if(!expectTokenType(cursor.get().next().value(), Token::SEMICOLON))
            return {nullptr};

        std::shared_ptr<NativeFunctionNode> f = std::make_shared<NativeFunctionNode>();
        f->m_signature = signatureOpt.value();
        f->m_function = Binder::getFunction(library, symbol);

        if(!f->m_function) {
            m_message.bindFunctionFail(library, symbol);
            return {nullptr};
        }

        funcs.push_back(f);
    }

    if(!expectTokenType(cursor.get().value(), Token::RIGHT_BRACE))
        return {nullptr};

    cursor.tryNext();

    return funcs;
}