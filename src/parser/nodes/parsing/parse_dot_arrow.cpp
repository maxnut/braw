#include "parser/nodes/file.hpp"
#include "parser/parser.hpp"
#include "../unary_operator.hpp"
#include <memory>

Result<std::shared_ptr<AST::UnaryOperatorNode>> Parser::parseDotArrow(TokenCursor& cursor, std::shared_ptr<AST::Node> left, const std::filesystem::path& path, std::shared_ptr<AST::FileNode> file) {
    if(!expectTokenType(cursor.get().value(), Token::OPERATOR))
        return unexpectedTokenExpectedType(cursor.value(), Token::OPERATOR, path);

    std::shared_ptr<AST::UnaryOperatorNode> dotArrow = std::make_shared<AST::UnaryOperatorNode>();
    dotArrow->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    dotArrow->m_operator = cursor.get().next().value().m_value;
    dotArrow->m_data = cursor.get().next().value().m_value;
    dotArrow->m_operand = std::move(left);
    dotArrow->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};

    return dotArrow;
}