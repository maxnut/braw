#pragma once

#include "register.hpp"
#include <cstdint>

struct Address {
    Register m_base;
    uint64_t m_offset;
};