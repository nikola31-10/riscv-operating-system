#include "../h/Scheduler.hpp"
#include "../h/TCB.hpp"
#include "../h/MemoryAllocator.hpp"

Scheduler::Node* Scheduler::head = nullptr;
Scheduler::Node* Scheduler::tail = nullptr;

int Scheduler::put(TCB* thread)
{
    if (thread == nullptr) {
        return -1;
    }

    const size_t nodeBlocks =
            (sizeof(Node) + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;

    Node* node = reinterpret_cast<Node*>(
            MemoryAllocator::allocate(nodeBlocks)
    );

    if (node == nullptr) {
        return -1;
    }

    node->thread = thread;
    node->next = nullptr;

    if (tail == nullptr) {
        head = tail = node;
    } else {
        tail->next = node;
        tail = node;
    }

    return 0;
}

TCB* Scheduler::get()
{
    if (head == nullptr) {
        return nullptr;
    }

    Node* oldHead = head;
    TCB* thread = oldHead->thread;

    head = head->next;

    if (head == nullptr) {
        tail = nullptr;
    }

    MemoryAllocator::free(oldHead);

    return thread;
}