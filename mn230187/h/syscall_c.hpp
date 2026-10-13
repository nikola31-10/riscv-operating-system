#pragma once

#include "../lib/hw.h"


class _thread;
using thread_t = _thread*;

void* mem_alloc(size_t size);
int mem_free(void* ptr);

int thread_create(
        thread_t* handle,
        void (*start_routine)(void*),
        void* arg
);

int thread_exit();
void thread_dispatch();
//void putc(char c);


class _sem;
using sem_t = _sem*;

int sem_open(sem_t* handle, unsigned init);
int sem_close(sem_t handle);
int sem_wait(sem_t id);
int sem_signal(sem_t id);
int time_sleep(time_t time);
int sem_wait_n(sem_t id, unsigned n);
int sem_signal_n(sem_t id, unsigned n);

char getc();
void putc(char c);

