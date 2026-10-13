#pragma once

class TCB;

class Scheduler {
private:
    struct Node {
        TCB* thread;
        Node* next;
    };

    static Node* head;
    static Node* tail;

public:
    static int put(TCB* thread);
    static TCB* get();
};