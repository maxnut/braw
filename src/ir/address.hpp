#pragma once

#include "register.hpp"
#include <cstdint>
#include <memory>

struct Address {
    std::shared_ptr<Register> m_base;
    uint64_t m_offset;
};