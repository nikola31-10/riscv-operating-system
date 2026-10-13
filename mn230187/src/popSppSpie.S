# 1 "src/popSppSpie.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/riscv64-linux-gnu/include/stdc-predef.h" 1 3
# 32 "<command-line>" 2
# 1 "src/popSppSpie.S"
.global popSppSpie
.type popSppSpie, @function

popSppSpie:
    # Procitaj sstatus.
    csrr t0, sstatus

    # Ocisti SPP bit (bit 8), da sret predje u user mode.
    li t1, 0x100
    not t1, t1
    and t0, t0, t1

    # Postavi SPIE bit (bit 5).
    #ori t0, t0, 0x20

    # Vrati izmenjeni status.
    csrw sstatus, t0

    # Ovde ra jos uvek sadrzi adresu nastavka
    # neposredno iza poziva popSppSpie.
    csrw sepc, ra

    # pc = sepc i prelazak u user mode.
    sret
