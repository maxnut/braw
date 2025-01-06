#pragma once

#include <stddef.h>

class Memory {
public:
    template<typename T>
    void from(T from) {
        *(T*)m_data = from;
        m_size = sizeof(T);
    }

    template<typename T>
    T get() {
        return *(T*)m_data;
    }

public:
    void* m_data;
    size_t m_size;
};