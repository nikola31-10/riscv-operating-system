#pragma once

#include "../lib/hw.h"

class MemoryAllocator {
private:
    struct BlockHeader {
        size_t blockCount;
        BlockHeader* next;
    };

    static BlockHeader* freeHead;

public:
    static void initialize();

    // Prima broj blokova, ne broj bajtova.
    static void* allocate(size_t blockCount);

    static int free(void* ptr);
};