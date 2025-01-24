#pragma once

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
    RegisterType m_type = Count;
};