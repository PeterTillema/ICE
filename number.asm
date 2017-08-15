    ld    hl, (outputPtr)
    bit    prev_is_number, (iy+fExpression1)
    set    prev_is_number, (iy+fExpression1)
    jr    z, AddNumberToStack
ChangeLastNumberFromStack:
    dec    hl
    dec    hl
    dec    hl
    push    hl
        ld    hl, (hl)
        add    hl, hl                                            ;    HL * 10
        push    hl
        pop    de
        add    hl, hl
        add    hl, hl
        add    hl, de
        sub    a, t0                                            ;    HL + <number>
        ld    de, 0
        ld    e, a
        add    hl, de
    pop    de
    ex    de, hl
    ld    (hl), de
    jr    NumberStop
AddNumberToStack:
    ld    (hl), typeNumber
    inc    hl
    sub    a, t0                                                ;    new number
InsertAndUpdatePointer
    ld    de, 0
    ld    e, a
    ld    (hl), de
UpdatePointer:
    inc    hl
    inc    hl
    inc    hl
    ld    (outputPtr), hl
NumberStop:
    call    _IncFetch
    jr    MainLoop