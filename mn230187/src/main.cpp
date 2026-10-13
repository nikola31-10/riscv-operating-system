#include "../h/MemoryAllocator.hpp"
#include "../h/Riscv.hpp"
#include "../h/TCB.hpp"
#include "../h/syscall_c.hpp"
#include "../lib/console.h"

extern "C" void supervisorTrap();

/*
 * Funkcija iz test/userMain.cpp.
 * Nije extern "C", jer je definisana kao obična C++ funkcija.
 */
extern void userMain();

void userMainWrapper(void*)
{
    userMain();
}

extern "C" void main()
{
    MemoryAllocator::initialize();

    Riscv::w_stvec(
            reinterpret_cast<uint64>(&supervisorTrap)
    );

    /*
     * Predstavlja trenutno aktivnu kernel/main nit.
     */
    TCB::running = TCB::createThread(nullptr, nullptr);

    if (TCB::running == nullptr) {
        __putc('M');
        __putc('\n');

        while (true) {
        }
    }

    thread_t userMainHandle = nullptr;
    //TCB* userMainHandle = nullptr;

    if (thread_create(
            &userMainHandle,
            userMainWrapper,
            nullptr
    ) != 0) {

        __putc('U');
        __putc('E');
        __putc('\n');

        while (true) {
        }
    }

    /*
     * Scheduler dobija procesor i pokreće userMain nit.
     */
    while (true) {
        thread_dispatch();
    }

    //while (!reinterpret_cast<TCB*>(userMainHandle)->isFinished()) {
        //thread_dispatch();
    //}
    //return;

}