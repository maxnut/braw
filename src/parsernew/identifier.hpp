#pragma once

#include <string>

struct Identifier {
    Identifier() = default;
    Identifier(const std::string& name) : m_name(name) {}

    std::string m_name;
};