#pragma once

#include "../lib/hw.h"
#include "TCB.hpp"

class _sem
{
public:
    explicit _sem(unsigned init = 1);

    int wait();
    int signal();

    int wait(unsigned n);
    int signal(unsigned n);

    int close();

private:
    unsigned value;
    bool closed;

    struct BlockedNode
    {
        TCB* thread;
        unsigned requested;
        BlockedNode* next;
    };

    BlockedNode* head;
    BlockedNode* tail;

    int block(unsigned requested);
    //TCB* unblock();
    TCB* unblock(int result);
};