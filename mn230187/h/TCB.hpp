#pragma once

#include "../lib/hw.h"

class TCB {
public:
    using Body = void (*)(void*);

    struct Context {
        uint64 ra;
        uint64 sp;
    };

private:
    Body body;
    void* arg;

    uint64* stack;
    Context context;

    bool finished;
    bool blocked;

    time_t timeSlice;

    static void threadWrapper();

    int semaphoreResult;
    bool sleeping;

    static void idleBody(void*);

    static void yield(TCB* oldThread, TCB* newThread);

public:
    static TCB* running;

    TCB(Body body, void* arg, uint64* stack);

    static void dispatch();

    static TCB* createThread(Body body, void* arg);

    static int exit();

    static int create(
            TCB** handle,
            Body body,
            void* arg,
            uint64* stack
    );

    Body getBody() const;
    void* getArg() const;

    Context* getContext();

    bool isFinished() const;
    void setFinished(bool value);

    bool isBlocked() const;
    void setBlocked(bool value);

    time_t getTimeSlice() const;

    int getSemaphoreResult() const;
    void setSemaphoreResult(int result);

    bool isSleeping() const;
    void setSleeping(bool value);

    static TCB* idle;

    static int initializeIdle();

};