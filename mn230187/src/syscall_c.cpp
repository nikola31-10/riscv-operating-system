#include "../h/syscall_c.hpp"

void* mem_alloc(size_t size)
{
    if (size == 0) {
        return nullptr;
    }

    /*
     * C API prima broj bajtova.
     * ABI prima broj blokova.
     */
    const size_t blockCount =
            (size + MEM_BLOCK_SIZE - 1) /
            MEM_BLOCK_SIZE;

    register uint64 a0 asm("a0") = 0x01;
    register uint64 a1 asm("a1") = blockCount;

    asm volatile(
    "ecall"
    : "+r"(a0)
    : "r"(a1)
    : "memory"
    );

    return reinterpret_cast<void*>(a0);
}

int mem_free(void* ptr)
{
    register uint64 a0 asm("a0") = 0x02;
    register uint64 a1 asm("a1") =
            reinterpret_cast<uint64>(ptr);

    asm volatile(
    "ecall"
    : "+r"(a0)
    : "r"(a1)
    : "memory"
    );

    return static_cast<int>(a0);
}
int thread_create(
        thread_t* handle,
        void (*start_routine)(void*),
        void* arg
)
{
    if (handle == nullptr || start_routine == nullptr) {
        return -1;
    }

    void* stack = mem_alloc(DEFAULT_STACK_SIZE);

    if (stack == nullptr) {
        return -2;
    }

    /*
     * ABI očekuje pokazivač na poslednju lokaciju steka.
     * Pošto stek raste ka nižim adresama, prosleđujemo kraj prostora.
     */
    uint64 stackTop =
            reinterpret_cast<uint64>(stack) +
            DEFAULT_STACK_SIZE;

    stackTop &= ~static_cast<uint64>(0xF);

    register uint64 a0 asm("a0") = 0x11;
    register uint64 a1 asm("a1") =
            reinterpret_cast<uint64>(handle);
    register uint64 a2 asm("a2") =
            reinterpret_cast<uint64>(start_routine);
    register uint64 a3 asm("a3") =
            reinterpret_cast<uint64>(arg);
    register uint64 a4 asm("a4") = stackTop;

    asm volatile(
    "ecall"
    : "+r"(a0)
    : "r"(a1), "r"(a2), "r"(a3), "r"(a4)
    : "memory"
    );

    int result = static_cast<int>(a0);

    if (result < 0) {
        mem_free(stack);
    }

    return result;
}

int thread_exit()
{
    register uint64 a0 asm("a0") = 0x12;

    asm volatile(
    "ecall"
    : "+r"(a0)
    :
    : "memory"
    );

    return static_cast<int>(a0);
}

void thread_dispatch()
{
    register uint64 a0 asm("a0") = 0x13;

    asm volatile(
    "ecall"
    : "+r"(a0)
    :
    : "memory"
    );
}

void putc(char c)
{
    register uint64 a0 asm("a0") = 0x42;
    register uint64 a1 asm("a1") =
            static_cast<uint64>(static_cast<unsigned char>(c));

    asm volatile(
    "ecall"
    : "+r"(a0)
    : "r"(a1)
    : "memory"
    );
}
char getc()
{
    register uint64 a0 asm("a0") = 0x41;

    asm volatile(
    "ecall"
    : "+r"(a0)
    :
    : "memory"
    );

    return static_cast<char>(a0);
}
int sem_open(sem_t* handle, unsigned init)
{
    register uint64 a0 asm("a0") = 0x21;
    register uint64 a1 asm("a1") =
            reinterpret_cast<uint64>(handle);
    register uint64 a2 asm("a2") =
            static_cast<uint64>(init);

    asm volatile(
    "ecall"
    : "+r"(a0)
    : "r"(a1), "r"(a2)
    : "memory"
    );

    return static_cast<int>(a0);
}

int sem_close(sem_t handle)
{
    register uint64 a0 asm("a0") = 0x22;
    register uint64 a1 asm("a1") =
            reinterpret_cast<uint64>(handle);

    asm volatile(
    "ecall"
    : "+r"(a0)
    : "r"(a1)
    : "memory"
    );

    return static_cast<int>(a0);
}

int sem_wait(sem_t id)
{
    register uint64 a0 asm("a0") = 0x23;
    register uint64 a1 asm("a1") =
            reinterpret_cast<uint64>(id);

    asm volatile(
    "ecall"
    : "+r"(a0)
    : "r"(a1)
    : "memory"
    );

    return static_cast<int>(a0);
}

int sem_signal(sem_t id)
{
    register uint64 a0 asm("a0") = 0x24;
    register uint64 a1 asm("a1") =
            reinterpret_cast<uint64>(id);

    asm volatile(
    "ecall"
    : "+r"(a0)
    : "r"(a1)
    : "memory"
    );

    return static_cast<int>(a0);
}
int sem_wait_n(sem_t id, unsigned n)
{
    uint64 result;

    __asm__ volatile(
    "mv a1, %1\n"
    "mv a2, %2\n"
    "li a0, 0x25\n"
    "ecall\n"
    "mv %0, a0\n"
    : "=r"(result)
    : "r"(id), "r"(n)
    : "a0", "a1", "a2"
    );

    return static_cast<int>(result);
}

int sem_signal_n(sem_t id, unsigned n)
{
    uint64 result;

    __asm__ volatile(
    "mv a1, %1\n"
    "mv a2, %2\n"
    "li a0, 0x26\n"
    "ecall\n"
    "mv %0, a0\n"
    : "=r"(result)
    : "r"(id), "r"(n)
    : "a0", "a1", "a2"
    );

    return static_cast<int>(result);
}
int time_sleep(time_t time)
{
    if (time == 0) {
        return 0;
    }

    uint64 result;

    __asm__ volatile(
    "mv a1, %1\n"
    "li a0, 0x31\n"
    "ecall\n"
    "mv %0, a0\n"
    : "=r"(result)
    : "r"(time)
    : "a0", "a1"
    );

    return static_cast<int>(result);
}