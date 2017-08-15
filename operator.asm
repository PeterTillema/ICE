    ld    hl, precedence
    add    hl, bc
    ld    e, (hl)                                        ;    e = precedence of current token
    cp    a, tStore
    call    z, MoveStackEntryToOutput
CheckOperator:
    ld    hl, (stackPtr)
    ld    bc, stack
    or    a, a
    sbc    hl, bc
    jr    z, InsertBoolean
    add    hl, bc
    dec    hl
    dec    hl
    dec    hl
    dec    hl
    ld    a, (hl)
    cp    a, typeOperator
    jr    nz, InsertBoolean
    inc    hl
    ld    a, (hl)
    ld    hl, operators_booleans
    ld    bc, 15
    cpir
    ld    hl, precedence2
    add    hl, bc
    ld    a, (hl)                                        ;    a = precedence of last token on stack
    cp    a, e                                            ;    if a <= e then move
    jr    c, InsertBoolean
MoveBooleanFromStackToOutput:
    ld    a, e                                            ;    save for the precedence of current token
    ld    de, (outputPtr)
    ld    hl, (stackPtr)
    dec    hl
    dec    hl
    dec    hl
    dec    hl
    ld    (stackPtr), hl
    ldi
    ldi
    ldi
    ldi
    ld    (outputPtr), de
    ld    e, a
    jr    CheckOperator
InsertBoolean:
    ld    hl, (stackPtr)
    ld    (hl), typeOperator
    inc    hl
    ld    a, (tempToken)
    ld    (hl), a
    inc    hl
    inc    hl
    inc    hl
    ld    (stackPtr), hl