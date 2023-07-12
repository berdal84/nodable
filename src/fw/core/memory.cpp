#include "memory.h"

size_t& fw::memory::allocated_bytes() noexcept
{
    static size_t count = 0;
    return count;
}

bool fw::memory::leaked()
{
     return allocated_bytes() > 0;
}

void* operator new(size_t size)
{
    fw::memory::allocated_bytes() += size;
    return malloc(size);
}

void operator delete(void* ptr, size_t size) noexcept
{
    free(ptr);
    fw::memory::allocated_bytes() -= size;
}