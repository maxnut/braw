#include "parser/parser.hpp"
#include "lexer/lexer.hpp"
#include "../file.hpp"
#include "spdlog/fmt/bundled/format.h"
#include "utils.hpp"

Result<std::shared_ptr<AST::FileNode>> Parser::parseImport(TokenCursor& cursor, const std::filesystem::path& fpath, std::shared_ptr<AST::FileNode> file) {
    if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
        return unexpectedTokenExpectedType(cursor.value(), Token::KEYWORD, fpath);

    if(!expectTokenValue(cursor.get().value(), "import"))
        return unexpectedTokenExpectedValue(cursor.value(), "import", fpath);

    if(!expectTokenType(cursor.next().get().value(), Token::QUOTE))
        return unexpectedTokenExpectedType(cursor.value(), Token::QUOTE, fpath);

    if(!expectTokenType(cursor.next().get().value(), Token::STRING))
        return unexpectedTokenExpectedType(cursor.value(), Token::STRING, fpath);

    std::filesystem::path path = cursor.get().value().m_value;
    if(std::filesystem::exists(Utils::getStdPath()) && std::filesystem::exists(Utils::getStdPath() / "include" / path))
        path = Utils::getStdPath() / "include" / path;

    if(!std::filesystem::exists(path))
        return std::unexpected{ParseError{
            fmt::format("File {} not found", path.string()),
            path,
            cursor.get().value().m_line,
            cursor.get().value().m_column
        }};

    auto tokensOpt = Lexer::tokenize(path);
    if(!tokensOpt)
        return std::unexpected{ParseError{
            fmt::format("Failed to tokenize file"),
            path,
            cursor.get().value().m_line,
            cursor.get().value().m_column
        }};

    if(!expectTokenType(cursor.next().get().value(), Token::QUOTE))
        return unexpectedTokenExpectedType(cursor.value(), Token::QUOTE, fpath);

    cursor.tryNext();

    return Parser::parse(tokensOpt.value(), path);
}