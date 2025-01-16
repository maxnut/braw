#pragma once

#include "memory.hpp"

#include <stdint.h>
#include <array>

class Stack {
public:
    Stack() {
        m_head = m_memory.data();
    }

    Stack(const Stack& other) {
        m_memory = other.m_memory;
        m_head = m_memory.data() + ((uintptr_t)other.m_head - (uintptr_t)other.m_memory.data());
    }

    Stack& operator=(const Stack& other) {
        if (this != &other) {
            m_memory = other.m_memory;
            m_head = m_memory.data() + ((uintptr_t)other.m_head - (uintptr_t)other.m_memory.data());
        }
        return *this;
    }

    Memory push(size_t size) {
        // if(size == 0)
        //     spdlog::warn("push(0)");
        Memory m{m_head, size};
        m_head = (void*)((uintptr_t)m_head + size);
        return m;
    }

    void pop(size_t size) {
        // if(size == 0)
        //     spdlog::warn("pop(0)");
        m_head = (void*)((uintptr_t)m_head - size);
    }

    void setHead(void* head) {
        // if(head == m_head)
        //     spdlog::warn("setHead({})", head);
        m_head = head;
    }

    void* ptr(size_t loc) {
        return (void*)((uintptr_t)m_memory.data() + loc);
    }

    void* ptrHead(size_t loc) {
        return (void*)((uintptr_t)m_head - loc);
    }

    void* head() {
        return m_head;
    }

    uint8_t* data() {
        return m_memory.data();
    }

private:
    std::array<uint8_t, 262144> m_memory;
    void* m_head;
};