#pragma once

#include "braw_context.hpp"
#include "macro/node.hpp"
#include "parser/nodes/file.hpp"
#include "parser/nodes/macro.hpp"
#include "parser/nodes/macro_call.hpp"
#include "parser/nodes/macro_parameter.hpp"
#include "parser/nodes/node.hpp"

#include <__expected/expected.h>
#include <filesystem>
#include <memory>
#include <unordered_map>

namespace Macro {

struct MacroError {
    std::string m_message;
    std::filesystem::path m_path;
    std::pair<uint32_t, uint32_t> m_rangeBegin;
    std::pair<uint32_t, uint32_t> m_rangeEnd;
};

struct EvaluatorContext {
    BrawContext& m_ctx;
    std::shared_ptr<AST::FileNode> m_file;
    std::unordered_map<std::string, std::shared_ptr<Node>> m_variables;
    AST::Node::Type m_rootType;
};

class Evaluator {
public:
    static std::optional<MacroError> processAST(std::shared_ptr<AST::FileNode> node, BrawContext& ctx) { return processAST(node, nullptr, nullptr, ctx); }
    static std::expected<std::shared_ptr<Node>, MacroError> evaluate(std::shared_ptr<AST::MacroNode> macro, std::shared_ptr<AST::MacroCallNode> macroCall, std::shared_ptr<AST::FileNode> file, EvaluatorContext& old);

private:
    static std::optional<MacroError> processAST(std::shared_ptr<AST::Node> node, std::shared_ptr<AST::Node>* replaceTarget, std::shared_ptr<AST::FileNode> file, BrawContext& ctx);
    static std::expected<std::shared_ptr<Node>, MacroError> deepClone(std::shared_ptr<AST::Node> node, EvaluatorContext& ctx);
    static std::expected<std::shared_ptr<Node>, MacroError> convertParameter(std::shared_ptr<AST::MacroParameterNode> param, EvaluatorContext& ctx);
    static std::expected<std::shared_ptr<Node>, MacroError> nodeFromType(const std::string type, std::shared_ptr<AST::Node> node, EvaluatorContext& ctx);

    static MacroError unexpectedParameterTypeExpected(std::shared_ptr<AST::MacroParameterNode> causer, AST::MacroParameterType expected, const std::filesystem::path& path);
    static MacroError unknownMacro(std::shared_ptr<AST::MacroCallNode> causer, const std::filesystem::path& path);
    static MacroError unknownFunction(std::shared_ptr<AST::MacroParameterFunctionNode> causer, const std::filesystem::path& path);
    static MacroError unknownType(std::shared_ptr<AST::Node> causer, const std::string& type, const std::filesystem::path& path);
    static MacroError notVariableDeclaration(std::shared_ptr<AST::Node> causer, const std::filesystem::path& path);
    static MacroError notFunctionDefinition(std::shared_ptr<AST::Node> causer, const std::filesystem::path& path);
    static MacroError expectedParamCount(std::shared_ptr<AST::Node> causer, int got, int expected, const std::filesystem::path& path);
};

}