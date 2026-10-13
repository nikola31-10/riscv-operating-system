#include "../h/Riscv.hpp"
#include "../lib/console.h"
#include "../h/MemoryAllocator.hpp"
#include "../h/TCB.hpp"
#include "../h/Semaphore.hpp"
#include "../h/new.hpp"
#include "../h/SleepList.hpp"





uint64 Riscv::r_scause()
{
    uint64 value;
    asm volatile("csrr %0, scause" : "=r"(value));
    return value;
}

uint64 Riscv::r_sepc()
{
    uint64 value;
    asm volatile("csrr %0, sepc" : "=r"(value));
    return value;
}

void Riscv::w_sepc(uint64 value)
{
    asm volatile("csrw sepc, %0" : : "r"(value));
}

uint64 Riscv::r_sstatus()
{
    uint64 value;
    asm volatile("csrr %0, sstatus" : "=r"(value));
    return value;
}

void Riscv::w_sstatus(uint64 value)
{
    asm volatile("csrw sstatus, %0" : : "r"(value));
}

void Riscv::w_stvec(uint64 value)
{
    asm volatile("csrw stvec, %0" : : "r"(value));
}
static void printHex(uint64 value)
{
    const char digits[] = "0123456789ABCDEF";

    __putc('0');
    __putc('x');

    for (int i = 15; i >= 0; i--) {
        const uint64 digit = (value >> (i * 4)) & 0xF;
        __putc(digits[digit]);
    }

    __putc('\n');
}
void Riscv::handleSupervisorTrap(TrapFrame* frame)
{
    const uint64 cause = r_scause();

    if (cause == 8 || cause == 9) {
        /*
         * Ove vrednosti pripadaju niti koja je izazvala ecall.
         *
         * Čuvaju se kao lokalne promenljive na njenom steku,
         * pa će nakon promene konteksta svaka nit nastaviti
         * sa svojim sačuvanim vrednostima.
         */
        const uint64 savedSepc = r_sepc() + 4;
        const uint64 savedSstatus = r_sstatus();

        const uint64 syscallCode = frame->a0;

        switch (syscallCode) {
            case 0x01: {
                const size_t blockCount =
                        static_cast<size_t>(frame->a1);

                void* result =
                        MemoryAllocator::allocate(blockCount);

                frame->a0 =
                        reinterpret_cast<uint64>(result);

                break;
            }

            case 0x02: {
                void* ptr =
                        reinterpret_cast<void*>(frame->a1);

                const int result =
                        MemoryAllocator::free(ptr);

                frame->a0 =
                        static_cast<uint64>(result);

                break;
            }

            case 0x11: {
                TCB** handle =
                        reinterpret_cast<TCB**>(frame->a1);

                TCB::Body body =
                        reinterpret_cast<TCB::Body>(frame->a2);

                void* arg =
                        reinterpret_cast<void*>(frame->a3);

                uint64* stackTop =
                        reinterpret_cast<uint64*>(frame->a4);

                uint64* stackStart =
                        reinterpret_cast<uint64*>(
                                reinterpret_cast<uint64>(stackTop) -
                                DEFAULT_STACK_SIZE
                        );

                const int result =
                        TCB::create(
                                handle,
                                body,
                                arg,
                                stackStart
                        );

                frame->a0 =
                        static_cast<uint64>(result);

                break;
            }

            case 0x12: {
                const int result = TCB::exit();

                frame->a0 =
                        static_cast<uint64>(result);

                break;
            }

            case 0x13: {
                TCB::dispatch();

                frame->a0 = 0;
                break;
            }

            case 0x21: {
                _sem** handle =
                        reinterpret_cast<_sem**>(frame->a1);

                unsigned init =
                        static_cast<unsigned>(frame->a2);

                if (handle == nullptr) {
                    frame->a0 = static_cast<uint64>(-1);
                    break;
                }

                const size_t blocks =
                        (sizeof(_sem) + MEM_BLOCK_SIZE - 1) /
                        MEM_BLOCK_SIZE;

                void* memory =
                        MemoryAllocator::allocate(blocks);

                if (memory == nullptr) {
                    frame->a0 = static_cast<uint64>(-2);
                    break;
                }

                _sem* sem =
                        new (memory) _sem(init);

                *handle = sem;
                frame->a0 = 0;
                break;
            }

            case 0x22: {
                _sem* sem =
                        reinterpret_cast<_sem*>(frame->a1);

                if (sem == nullptr) {
                    frame->a0 = static_cast<uint64>(-1);
                    break;
                }

                const int result = sem->close();

                /*
                 * Za sada ga ne oslobađamo odmah, jer blokirane
                 * niti tek treba da se vrate iz wait().
                 */
                frame->a0 = static_cast<uint64>(result);
                break;
            }

            case 0x23: {
                _sem* sem =
                        reinterpret_cast<_sem*>(frame->a1);

                frame->a0 =
                        sem != nullptr
                        ? static_cast<uint64>(sem->wait())
                        : static_cast<uint64>(-1);

                break;
            }

            case 0x24: {
                _sem* sem =
                        reinterpret_cast<_sem*>(frame->a1);

                frame->a0 =
                        sem != nullptr
                        ? static_cast<uint64>(sem->signal())
                        : static_cast<uint64>(-1);

                break;
            }
            case 0x25: {
                _sem* sem =
                        reinterpret_cast<_sem*>(frame->a1);

                unsigned n =
                        static_cast<unsigned>(frame->a2);

                frame->a0 =
                        sem != nullptr
                        ? static_cast<uint64>(sem->wait(n))
                        : static_cast<uint64>(-1);

                break;
            }

            case 0x26: {
                _sem* sem =
                        reinterpret_cast<_sem*>(frame->a1);

                unsigned n =
                        static_cast<unsigned>(frame->a2);

                frame->a0 =
                        sem != nullptr
                        ? static_cast<uint64>(sem->signal(n))
                        : static_cast<uint64>(-1);

                break;
            }
            case 0x31: {
                time_t time = static_cast<time_t>(frame->a1);

                if (time == 0) {
                    frame->a0 = 0;
                    break;
                }

                TCB* current = TCB::running;

                if (current == nullptr) {
                    frame->a0 = static_cast<uint64>(-1);
                    break;
                }

                const int result = SleepList::put(current, time);

                if (result != 0) {
                    frame->a0 = static_cast<uint64>(result);
                    break;
                }

                current->setSleeping(true);

                TCB::dispatch();

                frame->a0 = 0;
                break;
            }
            case 0x41:
            {
                char c = __getc();

                frame->a0 =
                        static_cast<uint64>(
                                static_cast<unsigned char>(c)
                        );

                break;
            }
            case 0x42:
            {
                __putc(static_cast<char>(frame->a1));
                frame->a0 = 0;
                break;
            }

            default:
                frame->a0 =
                        static_cast<uint64>(-1);
                break;
        }

        /*
         * Kada se nit posle context switch-a vrati ovde,
         * vraćamo baš njen sstatus i mesto nastavka.
         */
        w_sstatus(savedSstatus);
        w_sepc(savedSepc);
    }
    else if (
            cause ==
            SCAUSE_SUPERVISOR_SOFTWARE_INTERRUPT
            ) {
        /*
         * Otkucaj tajmera.
         */
        SleepList::tick();

        /*
         * Brišemo zahtev za supervisor software interrupt.
         * Bez ovoga bi se isti prekid odmah ponavljao.
         */
        mc_sip(SIP_SSIP);
    }
    else if (
            cause == SCAUSE_SUPERVISOR_EXTERNAL_INTERRUPT
            ) {
        console_handler();
    }
    else {
        __putc('X');
        __putc(':');
        __putc(' ');

        printHex(cause);

        while (true) {
        }
    }
}


uint64 Riscv::r_sie()
{
    uint64 value;
    asm volatile("csrr %0, sie" : "=r"(value));
    return value;
}

void Riscv::w_sie(uint64 value)
{
    asm volatile("csrw sie, %0" : : "r"(value));
}




void Riscv::ms_sie(uint64 mask)
{
    __asm__ volatile(
    "csrs sie, %0"
    :
    : "r"(mask)
    );
}

void Riscv::mc_sip(uint64 mask)
{
    __asm__ volatile(
    "csrc sip, %0"
    :
    : "r"(mask)
    );
}

void Riscv::enableTimerInterrupt()
{
    /*
     * Isključujemo sve prethodno dozvoljene prekide.
     */
    w_sie(0);

    /*
     * SSIE je potreban za timer.
     * SEIE je potreban za UART/tastaturu.
     */
    ms_sie(SIE_SSIE | SIE_SEIE);
}


extern "C" void handleSupervisorTrap(
        Riscv::TrapFrame* frame
)
{
    Riscv::handleSupervisorTrap(frame);
}

