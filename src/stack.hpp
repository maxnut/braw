#pragma once

#include "memory.hpp"

#include <array>

class Stack {
public:
    Stack() {
        m_head = m_memory.data();
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
    std::array<uint8_t, 5242> m_memory;
    void* m_head;
};