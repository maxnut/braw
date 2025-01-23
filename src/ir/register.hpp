#pragma once

#include "type_info.hpp"
#include <string>

enum RegisterType {
    Byte,
    Word,
    Dword,
    Qword,
    Single,
    Double,
    Count
};

struct Register {
    std::string m_id;
    TypeInfo m_type;
    RegisterType m_registerType = Count;
};