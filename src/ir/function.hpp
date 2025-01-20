#pragma once

#include "label.hpp"
#include "register.hpp"

struct Function {
    std::vector<Register> m_args;
    Label m_label;
};