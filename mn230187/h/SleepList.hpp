#ifndef SLEEP_LIST_HPP
#define SLEEP_LIST_HPP

#include "../lib/hw.h"

class TCB;

class SleepList
{
public:
    /*
     * Uspavljuje nit na zadati broj tick-ova.
     * Vraća 0 ako je uspešno, a negativnu vrednost pri grešci.
     */
    static int put(TCB* thread, time_t time);

    /*
     * Poziva se pri svakom prekidu tajmera.
     */
    static void tick();

private:
    struct SleepNode
    {
        TCB* thread;

        /*
         * Broj tick-ova u odnosu na prethodni element.
         */
        time_t relativeTime;

        SleepNode* next;
    };

    static SleepNode* head;
};

#endif