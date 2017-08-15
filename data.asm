operators_booleans:
    .db tStore, tAnd, tXor, tOr, tEQ, tLT, tGT, tLE, tGE, tNE, tMul, tDiv, tAdd, tSub, 0            ;    0 = temp +
operators_special:
    .db 0, 1, 2, 3, 4, 5, 5, 5, 5, 4, 6, 6, 6, 7
    
FunctionsWithReturnValue:
    .db tGetKey, trand, tLParen
FunctionsWithReturnValueArguments:
    .db tMean, tMin, tMax, tNot, tExtTok, tSqr, tSqrt
FunctionsWithReturnValueEnd:

FunctionsWithReturnValueStart:
    .dl functionRoot, functionSqrt, functionCE, functionNot, functionMax, functionMin, functionMean

FunctionsSingle:
    .db tOutput, tInput, tClLCD, tPause, tIf, tWhile, tRepeat, tDisp, tFor, tReturn, tVarOut, tLbl, tGoto, tii, tDet, tProg
FunctionsSingleEnd:

FunctionsSingleStart:
    .dl functionPrgm, functionC, functionSkipLine, functionGoto, functionLbl, functionCustom, functionReturn, functionFor
    .dl functionDisp, functionRepeat, functionWhile, functionIf, functionPause, functionClrHome, functionInput, functionOutput
    
operator_start:
    .dl SubNumberXXX,    SubVariableXXX,    SubChainPushXXX,    SubChainAnsXXX,    SubFunctionXXX,    SubError
    .dl AddNumberXXX,    AddVariableXXX,    AddChainPushXXX,    AddChainAnsXXX,    AddFunctionXXX,    AddError
    .dl DivNumberXXX,    DivVariableXXX,    DivChainPushXXX,    DivChainAnsXXX,    DivFunctionXXX,    DivError
    .dl MulNumberXXX,    MulVariableXXX,    MulChainPushXXX,    MulChainAnsXXX,    MulFunctionXXX,    MulError
    .dl NEQNumberXXX,    NEQVariableXXX,    NEQChainPushXXX,    NEQChainAnsXXX,    NEQFunctionXXX,    NEQError
    .dl GLETNumberXXX,   GLETVariableXXX,   GLETChainPushXXX,   GLETChainAnsXXX,   GLETFunctionXXX,   GLETError
    .dl XORANDNumberXXX, XORANDVariableXXX, XORANDChainPushXXX, XORANDChainAnsXXX, XORANDFunctionXXX, XORANDError
    .dl StoNumberXXX,    StoVariableXXX,    StoChainPushXXX,    StoChainAnsXXX,    StoFunctionXXX,    StoListXXX
    
CArguments:
    .dl CFunction0Args, CFunction1Arg, CFunction2Args, CFunction3Args, CFunction4Args, CFunction5Args, CFunction6Args
    
functionCustomStart:
    .dl functionExecHex, functionDefineSprite, functionCall, functionCompilePrgm, functionSetBASICVar, functionGetBASICVar
    
precedence:
    .db 7, 4,4,5,5,3,3,3,3,3,3,1, 1,  2,  0
    ;   t+ - + / * ≠ ≥ ≤ > < = or xor and →
precedence2:
    .db 0, 4,4,5,5,3,3,3,3,3,3,1, 1,  2,  6

lists:
    .dl L1, L2, L3, L4, L5, L6
    
hexadecimals:
    .db tF, tE, tD, tC, tB, tA, t9, t8, t7, t6, t5, t4, t3, t2, t1, t0
    
stackPtr:               .dl stack
outputPtr:              .dl output
programPtr:             .dl program
programNamesPtr:        .dl programNamesStack
labelPtr:               .dl labelStack
gotoPtr:                .dl gotoStack
programDataOffsetPtr:   .dl programDataOffsetStack
tempStringsPtr:         .dl tempStringsStack
tempListsPtr:           .dl tempListsStack
programDataDataPtr:     .dl programDataData
debugCodePtr:           .dl InsertDebugCode
amountOfPrograms        .db 0
openedParensE           .db 0
openedParensF           .db 0
amountOfArguments       .db 0
amountOfCRoutines       .db 0
amountOfEnds            .db 0
amountOfInput           .db 0
amountOfPause           .db 0
amountOfRoot            .db 0
ExprOutput              .db 0
ExprOutput2             .db 0
AmountOfSubPrograms     .db 0

ICEAppvar:
    .db AppVarObj, "ICEAPPV", 0
ICEProgram:
    .db ProtProgObj, "ICE", 0
ErrorMessageStandard:
    .db "Invalid arguments for '", 0
EndErrorMessage:
    .db "Too many Ends!", 0
GoodCompileMessage:
    .db "Succesfully compiled!", 0
NoProgramsMessage:
    .db "No programs found!", 0
InvalidTokenMessage:
    .db "Unsupported token!", 0
InvalidListArgumentMessage:
    .db "Only integers in lists supported!", 0
InvalidNameLengthMessage:
    .db "Program name too long!", 0
SameNameMessage:
    .db "Output name is the same as input name!", 0
WrongSpriteDataMessgae:
    .db "Invalid sprite data!", 0
FunctionFunctionMessage:
    .db "You can't use a function in a function!", 0
NotFoundMessage:
    .db "Program not found!", 0
ImplementMessage:
    .db "This function has not been implemented yet!", 0
SyntaxErrorMessage:
    .db "Invalid arguments entered!", 0
TooLargeLoopMessage:
    .db "Too large anonymous loop!", 0
UsedCodeMessage:
    .db "You can't use code before DefineSprite()!", 0
LineNumber:
    .db "Error on line ", 0
MismatchErrorMessage:
    .db "Mismatched parenthesis!", 0
UnknownMessage:
    .db "Something went wrong! Please report it!", 0
NotEnoughMem:
    .db "Not enough free RAM!", 0
LabelErrorMessage:
    .db "Can't find label ", 0
StartParseMessage:
    .db "Compiling program ", 0
ICEName:
    .db "ICE Compiler v1.5 - By Peter \"PT_\" Tillema", 0
PressKey:
    .db "[Press any key to exit]", 0
    
;MulTable:
;  1 + log2(x) + popcnt(x) - (popcnt(x) == 1)
; https://gist.github.com/jacobly0/049c51a353632d7fa284364f4b6244b6
;    .db 1 \ add hl, hl \ .db 0,0,0,0,0,0,0,0                                                                    ; 2
;    .db 4 \ push hl \ pop de \ add hl, hl \ add hl, de \ .db 0,0,0,0,0                                            ; 3
;    .db 2 \ add hl, hl \ add hl, hl \ .db 0,0,0,0,0,0,0                                ; 4
;    .db 5 \ push hl \ pop de \ add hl, hl \ add hl, hl \ add hl, de \ .db 0,0,0,0                    ; 5
;    .db 5 \ add hl, hl \ push hl \ pop de \ add hl, hl \ add hl, de \ .db 0,0,0,0                    ; 6
;    .db 6 \ push hl \ pop de \ add hl, hl \ add hl, de \ add hl, hl \ add hl, de \ .db 0,0,0            ; 7
;    .db 3 \ add hl, hl \ add hl, hl \ add hl, hl \ .db 0,0,0,0,0,0                            ; 8
;    .db 6 \ push hl \ pop de \ add hl, hl \ add hl, hl \ add hl, hl \ add hl, de \ .db 0,0,0            ; 9
;    .db 6 \ push hl \ pop de \ add hl, hl \ add hl, hl \ add hl, de \ add hl, hl \ .db 0,0,0            ; 10
;    .db 7 \ push hl \ pop de \ add hl, hl \ add hl, hl \ add hl, de \ add hl, hl \ add hl, de \ .db 0,0             ; 11
;    .db 6 \ add hl, hl \ add hl, hl \ push hl \ pop de \ add hl, hl \ add hl, de \ .db 0,0,0                        ; 12
;    .db 7 \ push hl \ pop de \ add hl, hl \ add hl, de \ add hl, hl \ add hl, hl \ add hl, de \ .db 0,0        ; 13
;    .db 7 \ push hl \ pop de \ add hl, hl \ add hl, de \ add hl, hl \ add hl, de \ add hl, hl \ .db 0,0             ; 14
;    .db 8 \    push hl \ pop de \ add hl, hl \ add hl, de \ add hl, hl \ add hl, de \ add hl, hl \ add hl, de \ .db 0    ; 15
;    .db 4 \ add hl, hl \ add hl, hl \ add hl, hl \ add hl, hl \ .db 0,0,0,0,0                                       ; 16
;    .db 7 \ push hl \ pop de \ add hl, hl \ add hl, hl \ add hl, hl \ add hl, hl \ add hl, de \ .db 0,0             ; 17
;    .db 7 \ push hl \ pop de \ add hl, hl \ add hl, hl \ add hl, hl \ add hl, de \ add hl, hl \ .db 0,0             ; 18
;    .db 8 \ push hl \ pop de \ add hl, hl \ add hl, hl \ add hl, hl \ add hl, de \ add hl, hl \ add hl, de \ .db 0    ; 19
;    .db 7 \ push hl \ pop de \ add hl, hl \ add hl, hl \ add hl, de \ add hl, hl \ add hl, hl \ .db 0,0             ; 20
    
InputRoutine:
    call    _ClrScrn
    call    _HomeUp
    xor     a, a
    ld      (ioPrompt), a
    ld      (curUnder), a
    call    _GetStringInput
    ld      hl, (editSym)
    call    _VarNameToOP1HL
    call    _ChkFindSym
    ld      a, (de)
    inc     de
    inc     de
    ld      b, a
    sbc     hl, hl
_:  push    bc
    add     hl, hl
    push    hl
    pop     bc
    add     hl, hl
    add     hl, hl
    add     hl, bc
    ld      a, (de)
    sub     a, t0
    ld      bc, 0
    ld      c, a
    add     hl, bc
    inc     de
    pop     bc
    djnz    -_
InputOffset = $+2
    ld      (ix+0), hl
    jp      _DeleteTempEditEqu
InputRoutineEnd:

RandRoutine:
    ld      hl, (ix+rand1)
    ld      de, (ix+rand2)
    ld      b, h
    ld      c, l
    add     hl, hl
    rl      e
    rl      d
    add     hl, hl
    rl      e
    rl      d
    inc     l
    add     hl, bc
    ld      (ix+rand1), hl
    adc     hl, de
    ld      (ix+rand2), hl
    ex      de, hl
    ld      hl, (ix+rand3)
    ld      bc, (ix+rand4)
    add     hl, hl
    rl      c
    rl      b
    ld      (ix+rand4), bc
    sbc     a, a
    and     a, 011000101b
    xor     a, l
    ld      l, a
    ld      (ix+rand3), hl
    ex      de, hl
    add     hl, bc
    ret
RandRoutineEnd:

DispNumberRoutine:
    ld      a, 18
    ld      (curCol), a
    call    _DispHL
    call    _NewLine
    
DispStringRoutine:
    xor     a, a
    ld      (curCol), a
    call    _PutS
    call    _NewLine

PauseRoutine:                       ; Time including call/ret: HL*48000000 + 44
    dec     hl                          ; 4
PauseRoutine2:
    ld      c, 110                      ; 8
_:  ld      b, 32                       ; 8
    djnz    $                       ; 13/8      411
    dec     c                           ; 4
    jr      nz, -_                      ; 13/8      47955
    or      a, a                        ; 4
    ld      de, -1                      ; 16
    add     hl, de                      ; 4
    jr      c, PauseRoutine2            ; 13/8
    ret                             ; 21
PauseRoutineEnd:

MeanRoutine:
    ld      ix, 0
    add     ix, sp
    add     hl, de
    push    hl
    rr      (ix-1)
    pop     hl
    rr      h
    rr      l
    ld      ix, L1+20000
    ret
MeanRoutineEnd:

KeypadRoutine:
    di
    ld      hl, 0F50000h
    ld      (hl), 2
    xor     a, a
_:  cp      a, (hl)
    jr      nz, -_
    ei
    ld      l, b
    ld      a, (hl)
    sbc     hl, hl
    and     a, c
    ret     z
    inc     l
    ret
KeypadRoutineEnd:

RootRoutine:
    di
    dec     sp      ; (sp) = ?
    push    hl      ; (sp) = ?uhl
    dec     sp      ; (sp) = ?uhl?
    pop     iy      ; (sp) = ?u, uix = hl?
    dec     sp      ; (sp) = ?u?
    pop     af      ; af = u?
    or      a, a
    sbc     hl, hl
    ex      de, hl   ; de = 0
    sbc     hl, hl   ; hl = 0
    ld      bc, 0C40h ; b = 12, c = 0x40
Sqrt24Loop:
    sub     a, c
    sbc     hl, de
    jr      nc, Sqrt24Skip
    add     a, c
    adc     hl, de
Sqrt24Skip:
    ccf
    rl      e
    rl      d
    add     iy, iy
    rla
    adc     hl, hl
    add     iy, iy
    rla
    adc     hl, hl
    djnz    Sqrt24Loop
    ex      de,hl
    ret
RootRoutineEnd:

XORANDData:
    ld      bc, -1
    add     hl, bc
    sbc     a, a
    ex      de, hl
    ld      d, a
    add     hl, bc
    sbc     a, a
XORANDSMC:
    and     a, d
    sbc     hl, hl
    and     a, 1
    ld      l, a
    
StoBASICVar:
    push    hl
    call    _ZeroOP1
    ld      hl, OP1+1
    ld      (hl), b
    call    _OP2Set0
    pop     hl
    add     hl, de
    or      a, a
    sbc     hl, de
    jr      z, +++_
    ld      b, 4
    ld      de, OP2+5
_:  ld      a, 10
    call    _DivHLByA
    ld      c, a
    ld      a, 10
    call    _DivHLByA
    add     a, a
    add     a, a
    add     a, a
    add     a, a
    add     a, c
    ld      (de), a
    dec     de
    djnz    -_
    ld      hl, OP2+1
    ld      (hl), $87
_:  ld      a, (OP2M)
    cp      a, $10
    jr      nc, +_
    ld      hl, OP2+5
    xor     a, a
    rld
    dec     hl
    rld
    dec     hl
    rld
    dec     hl
    rld
    dec     hl
    dec     (hl)
    jr      -_
_:  call    _ChkFindSym
    call    nc, _DelVarArc
    call    _CreateReal
    ld      hl, OP2
    ld      bc, 9
    ldir
    ret
StoBASICVarEnd:

GetBASICVar:
    call    _ZeroOP1
    ld      hl, OP1+1
    ld      (hl), b
    call    _FindSym
    call    nc, _RclVarSym
    call    _ConvOP1
    ex      de, hl
    ret
GetBASICVarEnd:

DebugCode:
    ld      hl, 0474F4Ch
    ld      (OP1+1), hl
    xor     a, a
    ld      (OP1+4), a
    call    _FindProgSym
    call    nc, _DelVarArc
    ld      hl, (ix+debugPtr)
    ld      de, debugStart
    or      a, a
    sbc     hl, de
    push    de
    push    hl
    call    _CreateProg
    pop     bc
    pop     hl
    inc     de
    inc     de
    ld      a, b
    or      a, c
    jr      z, $+4
    ldir
    ret
    ld      hl, debugStart
    ld      (ix+debugPtr), hl
DebugCodeEnd:

InsertDebugCode:
    ld      hl, (ix+debugPtr)
    ld      (hl), a
    inc     hl
_:  ld      a, (de)
    or      a, a
    jr      z, +_
    ld      (hl), a
    inc     hl
    inc     de
    jr      -_
_:  ld      (hl), tEnter
    inc     hl
    ld      (ix+debugPtr), hl
    ret
InsertDebugCodeEnd:

CData:
    ld      ix, L1+20000
    ld      hl, LibLoadAppVar - CData + UserMem
    call    _Mov9ToOP1
    ld      a, AppVarObj
    ld      (OP1), a
_:  call    _ChkFindSym
    jr      c, NotFound
    call    _ChkInRAM
    jr      nz, InArc
    call    _PushOP1
    call    _Arc_UnArc
    call    _PopOP1
    jr      -_
InArc:
    ex      de, hl
    ld      de, 9
    add     hl, de
    ld      e, (hl)
    add     hl, de
    inc     hl
    inc     hl
    inc     hl
    ld      de, RelocationStart - CData + UserMem
    jp      (hl)
NotFound:
    call    _ClrScrn
    call    _HomeUp
    ld      hl, MissingAppVar - CData + UserMem
    call    _PutS
    call    _NewLine
    jp    _PutS
MissingAppVar:
    .db "Need"
LibLoadAppVar:
    .db " LibLoad", 0
    .db "tiny.cc/clibs", 0
RelocationStart:
    .db 0C0h, "GRAPHX", 0, 5
CData2: