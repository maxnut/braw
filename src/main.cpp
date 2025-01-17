#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantic-analyzer/semantic_analyzer.hpp"
#include "execution-tree/builder/et_builder.hpp"

#include <spdlog/spdlog.h>
#include <args/args.hxx>

int main(int argc, char** argv) {
    spdlog::set_pattern("[%^%l%$] %v");
    spdlog::set_level(spdlog::level::debug);

    if(argc < 2) {
        spdlog::error("No file specified");
        return 1;
    }

    std::filesystem::path filepath(argv[1]);
    auto tokens = Lexer::tokenize(filepath);
    if(!tokens) {
        spdlog::error("Failed to tokenize file");
        return 1;
    }

    auto ast = Parser::parse(tokens.value());
    if(!ast) {
        spdlog::error("{}:{} {}", ast.error().m_line, ast.error().m_column, ast.error().m_message);
        return 1;
    }

    auto ctx = SemanticAnalyzer::analyze(ast.value().get());
    if(!ctx) {
        spdlog::error("{}:{} {}", ctx.error().m_rangeBegin.first, ctx.error().m_rangeBegin.second, ctx.error().m_message);
        return 1;
    }

    std::shared_ptr<FileNode> file = ETBuilder::buildFile(ast.value().get(), ctx.value());
    if(!file) {
        spdlog::error("Failed to build execution tree");
        return 1;
    }

    return 0;
}