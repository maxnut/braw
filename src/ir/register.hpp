#pragma once

#include "type_info.hpp"
#include <string>

enum RegisterType {
    Signed,
    Single,
    Double,
    Struct,
    Pointer,
    Count
};

struct Register {
    std::string m_id;
    TypeInfo m_type;
    RegisterType m_registerType = Count;
};