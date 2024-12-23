#include "parser/parser.hpp"

std::unique_ptr<EvaluatableNode> Parser::parsePrimary(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    if(Rules::isFunctionCall(cursor)) {

    }
    else if(Rules::isVariableAccess(cursor)) {

    }

    m_message.unexpectedToken(cursor.get().value());
    return nullptr;
}