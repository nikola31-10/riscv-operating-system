#include "../h/syscall_cpp.hpp"

Thread::Thread(void (*body)(void*), void* arg)
        : myHandle(nullptr),
          body(body),
          arg(arg)
{
}

Thread::Thread()
        : myHandle(nullptr),
          body(nullptr),
          arg(nullptr)
{
}

Thread::~Thread()
{
    /*
     * Za sada ne oslobađamo TCB ovde.
     * thread_exit završava nit, ali nemamo join niti
     * bezbedno brisanje TCB-a.
     */
}

int Thread::start()
{
    if (myHandle != nullptr) {
        return -1;
    }

    if (body != nullptr) {
        return thread_create(
                &myHandle,
                body,
                arg
        );
    }

    return thread_create(
            &myHandle,
            &Thread::threadWrapper,
            this
    );
}

void Thread::threadWrapper(void* thread)
{
    Thread* object =
            static_cast<Thread*>(thread);

    object->run();
}

void Thread::dispatch()
{
    thread_dispatch();
}

int Thread::sleep(time_t time)
{
    return time_sleep(time);
}
Semaphore::Semaphore(unsigned init)
        : myHandle(nullptr)
{
    sem_open(&myHandle, init);
}

Semaphore::~Semaphore()
{
    if (myHandle != nullptr) {
        sem_close(myHandle);
    }
}

int Semaphore::wait()
{
    return sem_wait(myHandle);
}

int Semaphore::signal()
{
    return sem_signal(myHandle);
}
PeriodicThread::PeriodicThread(time_t period)
        : Thread(),
          period(period)
{
}

void PeriodicThread::terminate()
{
    period = 0;
}

void PeriodicThread::run()
{
    while (period != 0) {
        periodicActivation();

        if (period == 0) {
            break;
        }

        if (Thread::sleep(period) < 0) {
            break;
        }
    }
}

char Console::getc()
{
    return ::getc();
}

void Console::putc(char c)
{
    ::putc(c);
}