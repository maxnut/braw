#include "parser/parser.hpp"

std::unique_ptr<FunctionInstructionNode> Parser::parseInstruction(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    if(Rules::isVariableDeclaration(cursor))
        return parseVariableDeclaration(file, cursor, ctx);

    return nullptr;
}