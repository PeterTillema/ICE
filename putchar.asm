_PrintChar_ASM:
    push    hl
TextXPos_ASM = $+1
    ld      bc, 0
    push    af
    push    af
    push    bc
    or      a, a
    sbc     hl, hl
    ld      l, a
    ld      de, DefaultCharSpacing_ASM
    add     hl, de
    ld      a, (hl)
    ld      (charwidth), a
    or      a, a
    sbc     hl, hl
    ld      l, a
    neg
    ld      (CharWidthDelta_ASM), a
    add     hl, bc
    ld      (TextXPos_ASM),hl
CharWidthDelta_ASM = $+1
    ld      de, -1
    ld      hl, lcdWidth
    add     hl, de
    ld      (line_change), hl
TextYPos_ASM    = $+1
    ld      l, 0
    ld      h, 160
    mlt     hl
    add     hl, hl
    ld      de, vRAM
    add     hl, de
    pop     de
    add     hl, de
    pop     af
    ex      de, hl
    or      a, a
    sbc     hl, hl
    sub     a, 23
    ld      l, a
    add     hl, hl
    add     hl, hl
    add     hl, hl
    ld      bc, DefaultTextData_ASM
    add     hl, bc
    ld      b, 8
iloop:
    push    bc
    ld      c, (hl)
charwidth = $+1
    ld      b, 0
    ex      de, hl
color = $+1
    ld      a, 0
cloop:
    sla     c
    jr      nc, +_
    ld      (hl), a
_:  inc     hl
    djnz    cloop
line_change =$+1
    ld      bc, 0
    add     hl, bc
    ex      de, hl
    inc     hl
    pop     bc
    djnz    iloop
    pop     af
    pop     hl
    ret

DefaultCharSpacing_ASM:
    ;      0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F
    .db    8,8,8,8,8,8,8,8,8,8,8,8,8,2,8,8    ;    0
    .db    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8    ;    1
    .db    3,4,6,8,8,8,8,5,5,5,8,7,4,7,3,8    ;    2
    .db    8,7,8,8,8,8,8,8,8,8,3,4,6,7,6,7    ;    3
    .db    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8    ;    4
    .db    8,8,8,8,8,8,8,8,8,8,8,5,8,5,8,8    ;    5
    .db    4,8,8,8,8,8,8,8,8,5,8,8,5,8,8,8    ;    6
    .db    8,8,8,8,7,8,8,8,8,8,8            ;    7
 
;-------------------------------------------------------------------------------
DefaultTextData_ASM:
Char000: .db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF    ;    _
Char024: .db $18,$3C,$7E,$18,$18,$18,$18,$00    ;    .
Char025: .db $18,$18,$18,$18,$7E,$3C,$18,$00    ;    .
Char026: .db $00,$18,$0C,$FE,$0C,$18,$00,$00    ;    .
Char027: .db $00,$30,$60,$FE,$60,$30,$00,$00    ;    .
Char028: .db $00,$18,$0C,$FE,$0C,$18,$00,$00    ;    ->
Char029: .db $00,$24,$66,$FF,$66,$24,$00,$00    ;    .
Char030: .db $00,$18,$3C,$7E,$FF,$FF,$00,$00    ;    .
Char031: .db $00,$FF,$FF,$7E,$3C,$18,$00,$00    ;    .
Char032: .db $00,$00,$00,$00,$00,$00,$00,$00    ;     
Char033: .db $C0,$C0,$C0,$C0,$C0,$00,$C0,$00    ;    !
Char034: .db $D8,$D8,$D8,$00,$00,$00,$00,$00    ;    "
Char035: .db $6C,$6C,$FE,$6C,$FE,$6C,$6C,$00    ;    #
Char036: .db $18,$7E,$C0,$7C,$06,$FC,$18,$00    ;    $
Char037: .db $00,$C6,$CC,$18,$30,$66,$C6,$00    ;    %
Char038: .db $38,$6C,$38,$76,$DC,$CC,$76,$00    ;    &
Char039: .db $30,$30,$60,$00,$00,$00,$00,$00    ;    '
Char040: .db $30,$60,$C0,$C0,$C0,$60,$30,$00    ;    (
Char041: .db $C0,$60,$30,$30,$30,$60,$C0,$00    ;    )
Char042: .db $00,$66,$3C,$FF,$3C,$66,$00,$00    ;    *
Char043: .db $00,$30,$30,$FC,$FC,$30,$30,$00    ;    +
Char044: .db $00,$00,$00,$00,$00,$60,$60,$C0    ;    ,
Char045: .db $00,$00,$00,$FC,$00,$00,$00,$00    ;    -
Char046: .db $00,$00,$00,$00,$00,$C0,$C0,$00    ;    .
Char047: .db $06,$0C,$18,$30,$60,$C0,$80,$00    ;    /
Char048: .db $7C,$CE,$DE,$F6,$E6,$C6,$7C,$00    ;    0
Char049: .db $30,$70,$30,$30,$30,$30,$FC,$00    ;    1
Char050: .db $7C,$C6,$06,$7C,$C0,$C0,$FE,$00    ;    2
Char051: .db $FC,$06,$06,$3C,$06,$06,$FC,$00    ;    3
Char052: .db $0C,$CC,$CC,$CC,$FE,$0C,$0C,$00    ;    4
Char053: .db $FE,$C0,$FC,$06,$06,$C6,$7C,$00    ;    5
Char054: .db $7C,$C0,$C0,$FC,$C6,$C6,$7C,$00    ;    6
Char055: .db $FE,$06,$06,$0C,$18,$30,$30,$00    ;    7
Char056: .db $7C,$C6,$C6,$7C,$C6,$C6,$7C,$00    ;    8
Char057: .db $7C,$C6,$C6,$7E,$06,$06,$7C,$00    ;    9
Char058: .db $00,$C0,$C0,$00,$00,$C0,$C0,$00    ;    :
Char059: .db $00,$60,$60,$00,$00,$60,$60,$C0    ;    ;
Char060: .db $18,$30,$60,$C0,$60,$30,$18,$00    ;    <
Char061: .db $00,$00,$FC,$00,$FC,$00,$00,$00    ;    =
Char062: .db $C0,$60,$30,$18,$30,$60,$C0,$00    ;    >
Char063: .db $78,$CC,$18,$30,$30,$00,$30,$00    ;    ?
Char064: .db $7C,$C6,$DE,$DE,$DE,$C0,$7E,$00    ;    @
Char065: .db $38,$6C,$C6,$C6,$FE,$C6,$C6,$00    ;    A
Char066: .db $FC,$C6,$C6,$FC,$C6,$C6,$FC,$00    ;    B
Char067: .db $7C,$C6,$C0,$C0,$C0,$C6,$7C,$00    ;    C
Char068: .db $F8,$CC,$C6,$C6,$C6,$CC,$F8,$00    ;    D
Char069: .db $FE,$C0,$C0,$F8,$C0,$C0,$FE,$00    ;    E
Char070: .db $FE,$C0,$C0,$F8,$C0,$C0,$C0,$00    ;    F
Char071: .db $7C,$C6,$C0,$C0,$CE,$C6,$7C,$00    ;    G
Char072: .db $C6,$C6,$C6,$FE,$C6,$C6,$C6,$00    ;    H
Char073: .db $7E,$18,$18,$18,$18,$18,$7E,$00    ;    I
Char074: .db $06,$06,$06,$06,$06,$C6,$7C,$00    ;    J
Char075: .db $C6,$CC,$D8,$F0,$D8,$CC,$C6,$00    ;    K
Char076: .db $C0,$C0,$C0,$C0,$C0,$C0,$FE,$00    ;    L
Char077: .db $C6,$EE,$FE,$FE,$D6,$C6,$C6,$00    ;    M
Char078: .db $C6,$E6,$F6,$DE,$CE,$C6,$C6,$00    ;    N
Char079: .db $7C,$C6,$C6,$C6,$C6,$C6,$7C,$00    ;    O
Char080: .db $FC,$C6,$C6,$FC,$C0,$C0,$C0,$00    ;    P
Char081: .db $7C,$C6,$C6,$C6,$D6,$DE,$7C,$06    ;    Q
Char082: .db $FC,$C6,$C6,$FC,$D8,$CC,$C6,$00    ;    R
Char083: .db $7C,$C6,$C0,$7C,$06,$C6,$7C,$00    ;    S
Char084: .db $FF,$18,$18,$18,$18,$18,$18,$00    ;    T
Char085: .db $C6,$C6,$C6,$C6,$C6,$C6,$FE,$00    ;    U
Char086: .db $C6,$C6,$C6,$C6,$C6,$7C,$38,$00    ;    V
Char087: .db $C6,$C6,$C6,$C6,$D6,$FE,$6C,$00    ;    W
Char088: .db $C6,$C6,$6C,$38,$6C,$C6,$C6,$00    ;    X
Char089: .db $C6,$C6,$C6,$7C,$18,$30,$E0,$00    ;    Y
Char090: .db $FE,$06,$0C,$18,$30,$60,$FE,$00    ;    Z
Char091: .db $F0,$C0,$C0,$C0,$C0,$C0,$F0,$00    ;    [
Char092: .db $C0,$60,$30,$18,$0C,$06,$02,$00    ;    \
Char093: .db $F0,$30,$30,$30,$30,$30,$F0,$00    ;    ]
Char094: .db $10,$38,$6C,$C6,$00,$00,$00,$00    ;    ^
Char095: .db $00,$00,$00,$00,$00,$00,$00,$FF    ;    _
Char096: .db $C0,$C0,$60,$00,$00,$00,$00,$00    ;    `
Char097: .db $00,$00,$7C,$06,$7E,$C6,$7E,$00    ;    a
Char098: .db $C0,$C0,$C0,$FC,$C6,$C6,$FC,$00    ;    b
Char099: .db $00,$00,$7C,$C6,$C0,$C6,$7C,$00    ;    c
Char100: .db $06,$06,$06,$7E,$C6,$C6,$7E,$00    ;    d
Char101: .db $00,$00,$7C,$C6,$FE,$C0,$7C,$00    ;    e
Char102: .db $1C,$36,$30,$78,$30,$30,$78,$00    ;    f
Char103: .db $00,$00,$7E,$C6,$C6,$7E,$06,$FC    ;    g
Char104: .db $C0,$C0,$FC,$C6,$C6,$C6,$C6,$00    ;    h
Char105: .db $60,$00,$E0,$60,$60,$60,$F0,$00    ;    i
Char106: .db $06,$00,$06,$06,$06,$06,$C6,$7C    ;    j
Char107: .db $C0,$C0,$CC,$D8,$F8,$CC,$C6,$00    ;    k
Char108: .db $E0,$60,$60,$60,$60,$60,$F0,$00    ;    l
Char109: .db $00,$00,$CC,$FE,$FE,$D6,$D6,$00    ;    m
Char110: .db $00,$00,$FC,$C6,$C6,$C6,$C6,$00    ;    n
Char111: .db $00,$00,$7C,$C6,$C6,$C6,$7C,$00    ;    o
Char112: .db $00,$00,$FC,$C6,$C6,$FC,$C0,$C0    ;    p
Char113: .db $00,$00,$7E,$C6,$C6,$7E,$06,$06    ;    q
Char114: .db $00,$00,$FC,$C6,$C0,$C0,$C0,$00    ;    r
Char115: .db $00,$00,$7E,$C0,$7C,$06,$FC,$00    ;    s
Char116: .db $30,$30,$FC,$30,$30,$30,$1C,$00    ;    t
Char117: .db $00,$00,$C6,$C6,$C6,$C6,$7E,$00    ;    u
Char118: .db $00,$00,$C6,$C6,$C6,$7C,$38,$00    ;    v
Char119: .db $00,$00,$C6,$C6,$D6,$FE,$6C,$00    ;    w
Char120: .db $00,$00,$C6,$6C,$38,$6C,$C6,$00    ;    x
Char121: .db $00,$00,$C6,$C6,$C6,$7E,$06,$FC    ;    y
Char122: .db $00,$00,$FE,$0C,$38,$60,$FE,$00    ;    z
Char123: .db $1C,$30,$30,$E0,$30,$30,$1C,$00    ;    {
Char124: .db $C0,$C0,$C0,$00,$C0,$C0,$C0,$00    ;    |
Char125: .db $E0,$30,$30,$1C,$30,$30,$E0,$00    ;    }
Char126: .db $76,$DC,$00,$00,$00,$00,$00,$00    ;    ~
Char127: .db $00,$10,$38,$6C,$C6,$C6,$FE,$00    ;    .
