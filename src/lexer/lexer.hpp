#pragma once

#include "../token.hpp"
#include "../cursor.hpp"

#include <filesystem>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <optional>

class Lexer {
public:
    static std::optional<std::vector<Token>> tokenize(std::filesystem::path path);
};