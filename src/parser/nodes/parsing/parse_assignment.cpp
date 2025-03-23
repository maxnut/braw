#include "parser/parser.hpp"
#include "../binary_operator.hpp"

Result<std::shared_ptr<AST::BinaryOperatorNode>> Parser::parseAssignment(TokenCursor& cursor, const std::filesystem::path& path, std::shared_ptr<AST::FileNode> file) {
    std::shared_ptr<AST::BinaryOperatorNode> assignment = std::make_shared<AST::BinaryOperatorNode>();
    assignment->m_operator = "=";

    assignment->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    auto leftOpt = parseExpression(cursor, path, file);
    if(!leftOpt)
        return std::unexpected{leftOpt.error()};
    assignment->m_left = std::move(leftOpt.value());

    if(!expectTokenType(cursor.get().next().value(), Token::ASSIGNMENT))
        return unexpectedTokenExpectedType(cursor.value(), Token::ASSIGNMENT, path);

    auto rightOpt = parseExpression(cursor, path, file);
    if(!rightOpt)
        return std::unexpected{rightOpt.error()};
    assignment->m_right = std::move(rightOpt.value());

    assignment->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};

    return assignment;
}