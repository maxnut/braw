#pragma once

#include "function.hpp"
#include <filesystem>
#include <vector>

struct File {
    std::vector<Function> m_functions;
    std::vector<Function*> m_externals;
    std::filesystem::path m_path;
};