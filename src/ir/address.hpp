#pragma once

#include "register.hpp"
#include "type_info.hpp"
#include <cstdint>
#include <memory>

struct Address {
    std::shared_ptr<Register> m_base;
    int64_t m_offset = 0;
    TypeInfo m_typeInfo;
    std::shared_ptr<Register> m_index = nullptr;
    int64_t m_scale = 0;
    int64_t m_scaleSize = 0;
};