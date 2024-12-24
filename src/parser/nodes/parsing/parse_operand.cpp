#include "parser/parser.hpp"

std::unique_ptr<EvaluatableNode> Parser::parseOperand(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    //TODO unary operators

    return parsePrimary(file, cursor, ctx);
}