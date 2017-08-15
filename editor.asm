OpenEditBuffer:
    ld      hl, OP1+1
    ld      de, progToEdit
    ld      bc, 8
    ldir
    ld      de, saveSScreen
    ld      hl, +_
    ld      bc, StopProgramEditor - +_
    ldir
    jp      saveSScreen
_:  ld      hl, UserMem
    ld      de, (asm_prgm_size)
    call    _DelMem
    ld      a, kPrgmEd
    call    _NewContext
    ld.sis  bc, (errOffset - 0D00000h)
    ld      hl, (editTail)
    ld      de, (editCursor)
    ldir
    ld      (editTail), hl
    ld      (editCursor), de
FindPreviousEnter:
    call    _BufLeft
    jr      z, AtTopOfProgram
    ld      a, e
    cp      a, tEnter
    jr      z, +_
    inc     bc
    jr      FindPreviousEnter
_:  call    _BufRight
AtTopOfProgram:
    push    bc
    call    _ClrWindow
    ld      hl, 0000001h
    ld      (curRow), hl
    ld      a, ':'
    call    _PutC
    call    _DispEOW
    pop     bc
_:  ld      a, b
    or      a, c
    jp      z, _Mon
    call    _CursorRight
    dec     bc
    jr      -_
StopProgramEditor: