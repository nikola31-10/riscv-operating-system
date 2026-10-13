#include "../lib/hw.h"
#include "../h/MemoryAllocator.hpp"

static size_t blocksForBytes(size_t size)
{
    return (size + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
}

void* operator new(size_t size)
{
    return MemoryAllocator::allocate(
            blocksForBytes(size)
    );
}

void operator delete(void* ptr) noexcept
{
    MemoryAllocator::free(ptr);
}

void* operator new[](size_t size)
{
    return MemoryAllocator::allocate(
            blocksForBytes(size)
    );
}

void operator delete[](void* ptr) noexcept
{
    MemoryAllocator::free(ptr);
}

void* operator new(size_t, void* ptr)
{
    return ptr;
}