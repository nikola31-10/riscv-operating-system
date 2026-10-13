#include "../h/Semaphore.hpp"
#include "../h/Scheduler.hpp"
#include "../h/MemoryAllocator.hpp"

_sem::_sem(unsigned init)
        : value(init),
          closed(false),
          head(nullptr),
          tail(nullptr)
{
}

int _sem::block(unsigned requested)
{
    TCB* current = TCB::running;

    if (current == nullptr) {
        return -1;
    }

    const size_t nodeBlocks =
            (sizeof(BlockedNode) + MEM_BLOCK_SIZE - 1) /
            MEM_BLOCK_SIZE;

    void* memory =
            MemoryAllocator::allocate(nodeBlocks);

    if (memory == nullptr) {
        return -2;
    }

    BlockedNode* node =
            reinterpret_cast<BlockedNode*>(memory);

    node->thread = current;
    node->requested = requested;
    node->next = nullptr;

    if (tail == nullptr) {
        head = tail = node;
    } else {
        tail->next = node;
        tail = node;
    }

    current->setSemaphoreResult(0);
    current->setBlocked(true);

    TCB::dispatch();

    /*
     * Nit se ovde vraća nakon signal-a ili close-a.
     */
    return current->getSemaphoreResult();
}

TCB* _sem::unblock(int result)
{
    if (head == nullptr) {
        return nullptr;
    }

    BlockedNode* node = head;
    head = head->next;

    if (head == nullptr) {
        tail = nullptr;
    }

    TCB* thread = node->thread;
    MemoryAllocator::free(node);

    thread->setSemaphoreResult(result);
    thread->setBlocked(false);
    Scheduler::put(thread);

    return thread;
}


int _sem::wait()
{
    return wait(1);
}

int _sem::signal()
{
    return signal(1);
}

int _sem::wait(unsigned n)
{
    if (closed) {
        return -1;
    }

    /*
     * Zahtev za nula jedinica je odmah uspešan.
     */
    if (n == 0) {
        return 0;
    }

    /*
     * Operacija je atomska:
     * ili uzimamo svih n jedinica ili se blokiramo
     * bez uzimanja bilo kog dela.
     */
    if (value >= n && head == nullptr) {
        value -= n;
        return 0;
    }

    return block(n);
}
int _sem::signal(unsigned n)
{
    if (closed) {
        return -1;
    }

    value += n;

    while (head != nullptr && value >= head->requested) {
        value -= head->requested;
        unblock(0);
    }

    return 0;
}

int _sem::close()
{
    if (closed) {
        return -1;
    }

    closed = true;

    while (head != nullptr) {
        unblock(-1);
    }

    return 0;
}

/*int _sem::signal(unsigned n)
{
    if (closed) {
        return -1;
    }

    value += n;


     //Poštujemo FIFO red.
     //Budimo prvu nit samo kada postoje sve jedinice
     //koje je ona tražila.

while (head != nullptr &&
value >= head->requested) {

const unsigned requested =
        head->requested;

value -= requested;

TCB* thread = unblock();

if (thread != nullptr) {
thread->setSemaphoreResult(0);
}
}

return 0;
}

int _sem::close()
{
    if (closed) {
        return -1;
    }

    closed = true;

    while (head != nullptr) {
        TCB* thread = unblock();

        if (thread != nullptr) {

             //Blokirani wait/wait_n nakon nastavka
             //vraća grešku.

            thread->setSemaphoreResult(-1);
        }
    }

    return 0;
}



TCB* _sem::unblock()
{
    if (head == nullptr) {
        return nullptr;
    }

    BlockedNode* node = head;
    head = head->next;

    if (head == nullptr) {
        tail = nullptr;
    }

    TCB* thread = node->thread;

    MemoryAllocator::free(node);

    thread->setBlocked(false);
    Scheduler::put(thread);

    return thread;
}*/
