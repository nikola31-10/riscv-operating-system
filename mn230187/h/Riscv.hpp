#pragma once

#include "../lib/hw.h"

class Riscv {
public:
    struct TrapFrame {
        uint64 ra;   // 0
        uint64 gp;   // 8
        uint64 tp;   // 16

        uint64 t0;   // 24
        uint64 t1;   // 32
        uint64 t2;   // 40

        uint64 s0;   // 48
        uint64 s1;   // 56

        uint64 a0;   // 64
        uint64 a1;   // 72
        uint64 a2;   // 80
        uint64 a3;   // 88
        uint64 a4;   // 96
        uint64 a5;   // 104
        uint64 a6;   // 112
        uint64 a7;   // 120

        uint64 s2;   // 128
        uint64 s3;   // 136
        uint64 s4;   // 144
        uint64 s5;   // 152
        uint64 s6;   // 160
        uint64 s7;   // 168
        uint64 s8;   // 176
        uint64 s9;   // 184
        uint64 s10;  // 192
        uint64 s11;  // 200

        uint64 t3;   // 208
        uint64 t4;   // 216
        uint64 t5;   // 224
        uint64 t6;   // 232
    };

    static uint64 r_scause();
    static uint64 r_sepc();
    static void w_sepc(uint64 value);

    static uint64 r_sstatus();
    static void w_sstatus(uint64 value);

    static void w_stvec(uint64 value);

    static void handleSupervisorTrap(TrapFrame* frame);

    static constexpr uint64 SSTATUS_SPP = 1UL << 8;
    static constexpr uint64 SSTATUS_SPIE = 1UL << 5;

    static uint64 r_sie();
    static void w_sie(uint64 value);

    static constexpr uint64 SCAUSE_SUPERVISOR_SOFTWARE_INTERRUPT =
            (1ULL << 63) | 1;

    static constexpr uint64 SIP_SSIP = 1ULL << 1;
    static constexpr uint64 SIE_SSIE = 1ULL << 1;
    static void enableTimerInterrupt();

    static void ms_sie(uint64 mask);
    static void mc_sip(uint64 mask);

    static constexpr uint64 SCAUSE_SUPERVISOR_EXTERNAL_INTERRUPT =
            (1ULL << 63) | 9ULL;

    static constexpr uint64 SIE_SEIE =
            1ULL << 9;


};