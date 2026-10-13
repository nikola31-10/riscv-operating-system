.global contextSwitch
.type contextSwitch, @function

contextSwitch:
    # a0 pokazuje na kontekst stare niti
    # a1 pokazuje na kontekst nove niti

    # Sačuvaj ra i sp stare niti.
    sd ra, 0(a0)
    sd sp, 8(a0)

    # Učitaj ra i sp nove niti.
    ld ra, 0(a1)
    ld sp, 8(a1)

    # pc dobija vrednost iz ra.
    ret