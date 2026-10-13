#include "../h/SleepList.hpp"
#include "../h/TCB.hpp"
#include "../h/Scheduler.hpp"
#include "../h/MemoryAllocator.hpp"

SleepList::SleepNode* SleepList::head = nullptr;

int SleepList::put(TCB* thread, time_t time)
{
    if (thread == nullptr || time == 0) {
        return -1;
    }

    const size_t blocks =
            (sizeof(SleepNode) + MEM_BLOCK_SIZE - 1) /
            MEM_BLOCK_SIZE;

    void* memory = MemoryAllocator::allocate(blocks);

    if (memory == nullptr) {
        return -2;
    }

    SleepNode* node =
            reinterpret_cast<SleepNode*>(memory);

    node->thread = thread;
    node->relativeTime = time;
    node->next = nullptr;

    if (head == nullptr) {
        head = node;
        return 0;
    }

    if (time < head->relativeTime) {
        head->relativeTime -= time;

        node->next = head;
        head = node;

        return 0;
    }

    time_t remaining = time;
    SleepNode* current = head;

    remaining -= current->relativeTime;

    while (current->next != nullptr &&
           remaining >= current->next->relativeTime) {

        current = current->next;
        remaining -= current->relativeTime;
    }

    node->relativeTime = remaining;
    node->next = current->next;

    if (current->next != nullptr) {
        current->next->relativeTime -= remaining;
    }

    current->next = node;

    return 0;
}

void SleepList::tick()
{
    if (head == nullptr) {
        return;
    }

    /*
     * Smanjujemo samo prvi element jer lista koristi
     * relativna vremena.
     */
    if (head->relativeTime > 0) {
        head->relativeTime--;
    }

    /*
     * Moguće je da više niti treba da se probudi
     * u istom tick-u.
     */
    while (head != nullptr &&
           head->relativeTime == 0) {

        SleepNode* node = head;
        head = head->next;

        TCB* thread = node->thread;

        MemoryAllocator::free(node);

        thread->setSleeping(false);
        Scheduler::put(thread);
    }
}