#include "parser/parser.hpp"

std::unique_ptr<FunctionInstructionNode> Parser::parseInstruction(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    std::unique_ptr<FunctionInstructionNode> instruction = nullptr;

    Rules::InstructionType instructionType = Rules::getInstructionType(cursor);
    
    if(Rules::isVariableDeclaration(cursor))
        instruction = parseVariableDeclaration(file, cursor, ctx);
    else if(Rules::isAssignment(cursor))
        instruction = parseAssignment(file, cursor, ctx);
    else if(Rules::isReturn(cursor))
        instruction = parseReturn(file, cursor, ctx);
    else if(Rules::isIf(cursor))
        instruction = parseIf(file, cursor, ctx);
    else
        instruction = parseExpression(file, cursor, ctx);

    switch(instructionType) {
        case Rules::InstructionType::VARIABLE_DECLARATION:
        case Rules::InstructionType::ASSIGNMENT:
        case Rules::InstructionType::RETURN: {
            if(!expectTokenType(cursor.get().next().value(), Token::SEMICOLON))
                return nullptr;

            break;
        }
        default:
            break;
    }
    
    return instruction;
}