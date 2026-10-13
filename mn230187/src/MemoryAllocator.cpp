#include "../h/MemoryAllocator.hpp"

MemoryAllocator::BlockHeader* MemoryAllocator::freeHead = nullptr;

void MemoryAllocator::initialize()
{
    uint64 heapStart = reinterpret_cast<uint64>(HEAP_START_ADDR);
    uint64 heapEnd = reinterpret_cast<uint64>(HEAP_END_ADDR);

    freeHead = reinterpret_cast<BlockHeader*>(heapStart);

    freeHead->blockCount =
            (heapEnd - heapStart) / MEM_BLOCK_SIZE;

    freeHead->next = nullptr;
}

void* MemoryAllocator::allocate(size_t blockCount)
{
    if (blockCount == 0) {
        return nullptr;
    }

    // Treba nam još jedan blok za zaglavlje alociranog segmenta.
    size_t requiredBlocks = blockCount + 1;

    BlockHeader* previous = nullptr;
    BlockHeader* current = freeHead;

    while (current != nullptr) {
        if (current->blockCount >= requiredBlocks) {

            if (current->blockCount == requiredBlocks) {
                // Uzimamo ceo slobodan segment.
                if (previous == nullptr) {
                    freeHead = current->next;
                } else {
                    previous->next = current->next;
                }
            } else {
                // Odvajamo početak segmenta, a ostatak ostaje slobodan.
                uint64 currentAddress =
                        reinterpret_cast<uint64>(current);

                BlockHeader* remaining =
                        reinterpret_cast<BlockHeader*>(
                                currentAddress +
                                requiredBlocks * MEM_BLOCK_SIZE
                        );

                remaining->blockCount =
                        current->blockCount - requiredBlocks;

                remaining->next = current->next;

                if (previous == nullptr) {
                    freeHead = remaining;
                } else {
                    previous->next = remaining;
                }

                current->blockCount = requiredBlocks;
            }

            current->next = nullptr;

            // Korisniku vraćamo adresu posle jednog bloka zaglavlja.
            return reinterpret_cast<void*>(
                    reinterpret_cast<uint64>(current) +
                    MEM_BLOCK_SIZE
            );
        }

        previous = current;
        current = current->next;
    }

    return nullptr;
}

int MemoryAllocator::free(void* ptr)
{
    if (ptr == nullptr) {
        return -1;
    }

    uint64 ptrAddress = reinterpret_cast<uint64>(ptr);

    // Zaglavlje alociranog segmenta nalazi se jedan blok pre ptr-a.
    BlockHeader* block = reinterpret_cast<BlockHeader*>(
            ptrAddress - MEM_BLOCK_SIZE
    );

    BlockHeader* previous = nullptr;
    BlockHeader* current = freeHead;

    // Tražimo mesto u sortiranoj listi po adresama.
    while (current != nullptr &&
           reinterpret_cast<uint64>(current) <
           reinterpret_cast<uint64>(block)) {
        previous = current;
        current = current->next;
    }

    block->next = current;

    if (previous == nullptr) {
        freeHead = block;
    } else {
        previous->next = block;
    }

    // Spoji block sa narednim segmentom ako su susedni.
    if (block->next != nullptr) {
        uint64 blockEnd =
                reinterpret_cast<uint64>(block) +
                block->blockCount * MEM_BLOCK_SIZE;

        if (blockEnd == reinterpret_cast<uint64>(block->next)) {
            block->blockCount += block->next->blockCount;
            block->next = block->next->next;
        }
    }

    // Spoji prethodni segment sa block-om ako su susedni.
    if (previous != nullptr) {
        uint64 previousEnd =
                reinterpret_cast<uint64>(previous) +
                previous->blockCount * MEM_BLOCK_SIZE;

        if (previousEnd == reinterpret_cast<uint64>(block)) {
            previous->blockCount += block->blockCount;
            previous->next = block->next;
        }
    }

    return 0;
}