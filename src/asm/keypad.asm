.assume adl = 1
segment data
.def _KeypadData

_KeypadData:
    di
    ld      hl, 0F50200h
    ld      (hl), h
    xor     a, a
CheckKeyPressLoop:
    cp      a, (hl)
    jr      nz, CheckKeyPressLoop
    ei
    ld      l, b
    ld      a, (hl)
    sbc     hl, hl
    and     a, c
    ret     z
    inc     l
    ret
