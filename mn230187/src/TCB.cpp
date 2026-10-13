#include "../h/TCB.hpp"
#include "../h/Scheduler.hpp"
#include "../h/MemoryAllocator.hpp"
#include "../h/Riscv.hpp"
#include "../h/syscall_c.hpp"
#include "../h/new.hpp"


extern "C" void popSppSpie();
extern "C" void contextSwitch(
        TCB::Context* oldContext,
        TCB::Context* newContext
);


TCB* TCB::running = nullptr;
TCB* TCB::idle = nullptr;

TCB::TCB(Body body, void* arg, uint64* stack)
        : body(body),
          arg(arg),
          stack(stack),
          finished(false),
          blocked(false),
          timeSlice(DEFAULT_TIME_SLICE),
          semaphoreResult(0),
          sleeping(false)
{
    /*
     * Ako body postoji, ovo je nova korisnička nit.
     * Prilikom prvog context switch-a ret će skočiti
     * u threadWrapper().
     */
    context.ra = body != nullptr
                 ? reinterpret_cast<uint64>(&TCB::threadWrapper)
                 : 0;

    if (stack != nullptr) {
        context.sp = reinterpret_cast<uint64>(
                stack + DEFAULT_STACK_SIZE / sizeof(uint64)
        );

        // Stek mora biti poravnat na 16 bajtova.
        context.sp &= ~static_cast<uint64>(0xF);
    } else {
        context.sp = 0;
    }
}
TCB* TCB::createThread(Body body, void* arg)
{
    uint64* stack = nullptr;

    if (body != nullptr) {
        const size_t stackBlocks =
                DEFAULT_STACK_SIZE / MEM_BLOCK_SIZE;

        stack = reinterpret_cast<uint64*>(
                MemoryAllocator::allocate(stackBlocks)
        );

        if (stack == nullptr) {
            return nullptr;
        }
    }

    const size_t tcbBlocks =
            (sizeof(TCB) + MEM_BLOCK_SIZE - 1) /
            MEM_BLOCK_SIZE;

    void* tcbMemory =
            MemoryAllocator::allocate(tcbBlocks);

    if (tcbMemory == nullptr) {
        if (stack != nullptr) {
            MemoryAllocator::free(stack);
        }

        return nullptr;
    }

    TCB* thread =
            new (tcbMemory) TCB(body, arg, stack);

    /*
     * Ako body postoji, nit je korisnička/radna nit
     * i treba da uđe u red spremnih.
     *
     * Ako je body == nullptr, to je glavni TCB
     * koji predstavlja trenutno main izvršavanje.
     */
    if (body != nullptr) {
        if (Scheduler::put(thread) < 0) {
            MemoryAllocator::free(stack);
            MemoryAllocator::free(thread);
            return nullptr;
        }
    }

    return thread;
}

void TCB::dispatch()
{
    TCB* old = running;

    /*
     * Obična spremna nit vraća se u Scheduler.
     * Idle se nikada ne stavlja u ready red.
     */
    if (old != nullptr &&
        old != idle &&
        !old->isFinished() &&
        !old->isBlocked() &&
        !old->isSleeping()) {

        Scheduler::put(old);
    }

    TCB* next = Scheduler::get();

    /*
     * Ako nema običnih spremnih niti,
     * izvršava se idle nit.
     */
    if (next == nullptr) {
        next = idle;
    }

    if (next == nullptr) {
        /*
         * Ovo je moguće samo ako idle nije inicijalizovan.
         */
        running = old;
        return;
    }

    running = next;

    yield(old, running);
}
int TCB::exit()//zavrsavanje niti
{
    if (running == nullptr || running == idle) {
        return -1;
    }

    running->finished = true;
    dispatch();

    return 0;
}

void TCB::threadWrapper()
{
    popSppSpie();

    if (running != nullptr && running->body != nullptr) {
        running->body(running->arg);
    }

    /*
     * Nalazimo se u user režimu, zato završavanje niti
     * mora da ide preko sistemskog poziva.
     */
    thread_exit();

    /*
     * Uspešan thread_exit se nikada ne vraća.
     */
    while (true) {
    }
}
int TCB::create(
        TCB** handle,
        Body body,
        void* arg,
        uint64* stack
)
{
    if (handle == nullptr || body == nullptr || stack == nullptr) {
        return -1;
    }

    const size_t tcbBlocks =
            (sizeof(TCB) + MEM_BLOCK_SIZE - 1) /
            MEM_BLOCK_SIZE;

    void* tcbMemory =
            MemoryAllocator::allocate(tcbBlocks);

    if (tcbMemory == nullptr) {
        return -2;
    }

    TCB* thread =
            new (tcbMemory) TCB(body, arg, stack);

    if (Scheduler::put(thread) < 0) {
        MemoryAllocator::free(thread);
        return -3;
    }

    *handle = thread;
    return 0;
}
TCB::Body TCB::getBody() const
{
    return body;
}

void* TCB::getArg() const
{
    return arg;
}

TCB::Context* TCB::getContext()
{
    return &context;
}

bool TCB::isFinished() const
{
    return finished;
}

void TCB::setFinished(bool value)
{
    finished = value;
}

bool TCB::isBlocked() const
{
    return blocked;
}

void TCB::setBlocked(bool value)
{
    blocked = value;
}

time_t TCB::getTimeSlice() const
{
    return timeSlice;
}
int TCB::getSemaphoreResult() const
{
    return semaphoreResult;
}

void TCB::setSemaphoreResult(int result)
{
    semaphoreResult = result;
}

bool TCB::isSleeping() const
{
    return sleeping;
}

void TCB::setSleeping(bool value)
{
    sleeping = value;
}

void TCB::idleBody(void*)
{
    while (true) {
        thread_dispatch(); //posto nemam asinhrono, onda mora dobrovoljno da preda precsor, tj da ispituje
    }
}
int TCB::initializeIdle()
{
    if (idle != nullptr) {
        return 0;
    }

    const size_t stackBlocks =
            (DEFAULT_STACK_SIZE + MEM_BLOCK_SIZE - 1) /
            MEM_BLOCK_SIZE;

    uint64* idleStack = reinterpret_cast<uint64*>(
            MemoryAllocator::allocate(stackBlocks)
    );

    if (idleStack == nullptr) {
        return -1;
    }

    const size_t tcbBlocks =
            (sizeof(TCB) + MEM_BLOCK_SIZE - 1) /
            MEM_BLOCK_SIZE;

    void* tcbMemory =
            MemoryAllocator::allocate(tcbBlocks);

    if (tcbMemory == nullptr) {
        MemoryAllocator::free(idleStack);
        return -2;
    }

    idle = new (tcbMemory) TCB(
            &TCB::idleBody,
            nullptr,
            idleStack
    );

    return 0;
}
void TCB::yield(TCB* oldThread, TCB* newThread)
{
    if (oldThread == nullptr ||
        newThread == nullptr ||
        oldThread == newThread) {
        return;
    }

    contextSwitch(
            oldThread->getContext(),
            newThread->getContext()
    );
}