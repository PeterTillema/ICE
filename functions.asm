InsertBCDEAHL:
	push	hl
	push	bc
	pop	hl
	call	InsertHL
	ex	de, hl
	call	InsertHL
	pop	hl
InsertAHL:
	call	InsertA
InsertHL:
	push	de
	ex	de, hl
	ld	hl, (programPtr)
	ld	(hl), de
	inc	hl
	inc	hl
	inc	hl
	ld	(programPtr), hl
	pop	de
	ret
    
InsertHIXC:
	ld	hl, 00027DDh
	ld	b, 3
	mlt	bc
	ld	a, c
	call	_SetHLUToA
	jp	InsertHL		; ld hl, (ix+*)
InsertIXE:
	ld	hl, 00017DDh
	ld	d, 3
	mlt	de
	ld	a, e
	call	_SetHLUToA
	jp	InsertHL		; ld de, (ix+*)
InsertIXC:
	ld	hl, 00007DDh
	ld	d, 3
	mlt	de
	ld	a, e
	call	_SetHLUToA
	jp	InsertHL		; ld bc, (ix+*)

InsertBCDEHL:
	push	hl
	push	bc
	pop	hl
	call	InsertHL
	pop	hl
	jr	InsertDEHL
InsertADEHL:
	call	InsertA
InsertDEHL:
	ex	de, hl
	call	InsertHL
	ex	de, hl
	jr	InsertHL

InsertE:
	ld	a,   e
InsertA:
	push	hl
	ld	hl, (programPtr)
	ld	(hl), a
	inc	hl
	ld	(programPtr), hl
	pop	hl
	ret
    
InsertProgramPtrToDataOffset:
	push	de
	ld	hl, (programDataOffsetPtr)
	ld	de, (programPtr)
	ld	(hl), de
	inc	hl
	inc	hl
	inc	hl
	ld	(programDataOffsetPtr), hl
	pop	de
	ret
    
GetFunction:
	push	bc
	push	de
	call	GetRightFunction
	pop	de
	pop	bc
	ret
    
GetRightFunction:
	ld	h, a
	ld	a, b
	or	a, a
	jp	z, GetFunctionBC
	dec	a
	jp	z, GetFunctionDE
GetFunctionHL:
	ld	a, h
	cp	a, tGetKey
	jr	nz, +_
	ld	hl, _GetCSC
	call	InsertCallHL		; call _GetCSC
	ld	a, 0B7h
	ld	hl, 06F62EDh
	jp	InsertAHL		; or a \ sbc hl, hl \ ld l, a
_:	cp	a, trand
	jp	nz, InvalidTokenError
	bit	need_push, (iy+myFlags)
	jr	z, $+8
	ld	a, 0D5h
	call	InsertA			; push de
	ld	a, 0CDh
	call	InsertA			; call *
	call	InsertProgramPtrToDataOffset
	bit	has_already_rand, (iy+myFlags5)
	jr	nz, +_
	ld	hl, (programDataDataPtr)
	push	hl
	pop	de
	ld	(RandStartData), hl
	call	InsertHL		; call XXXXXX
	ld	hl, RandRoutine
	ld	bc, RandRoutineEnd - RandRoutine
	ldir
	ld	(programDataDataPtr), de
	set	has_already_rand, (iy+myFlags5)
	bit	need_push, (iy+myFlags)
	ret	z
	ld	a, 0D1h
	jp	InsertA			; pop de
_:	ld	hl, (RandStartData)
	call	InsertHL		; call XXXXXX
	bit	need_push, (iy+myFlags)
	ret	z
	ld	a, 0D1h
	jp	InsertA			; pop de
GetFunctionDE:
	ld	a, h
	cp	a, tGetKey
	jr	nz, +_
	ld	a, 0E5h
	bit	need_push, (iy+myFlags)
	call	nz, InsertA		; push hl
	ld	hl, _GetCSC
	call	InsertCallHL		; call _GetCSC
	bit	need_push, (iy+myFlags)
	jr	z, $+8
	ld	a, 0E1h
	call	InsertA			; pop hl
	ld	a, 011h
	call	InsertA			; ld de, *
	xor	a, a
	ld	hl, 05F0000h
	jp	InsertAHL		; ld de, 0 \ ld e, a
_:	cp	a, trand
	jp	nz, InvalidTokenError
	ld	a, 0E5h
	bit	need_push, (iy+myFlags)
	call	nz, InsertA		; push hl
	ld	a, 0CDh
	call	InsertA			; call *
	call	InsertProgramPtrToDataOffset
	bit	has_already_rand, (iy+myFlags5)
	jr	nz, +_
	ld	hl, (programDataDataPtr)
	ld	(RandStartData), hl
	push	hl
	pop	de
	call	InsertHL		; call XXXXXX
	ld	hl, RandRoutine
	ld	bc, RandRoutineEnd - RandRoutine
	ldir
	ld	(programDataDataPtr), de
	set	has_already_rand, (iy+myFlags5)
	jr	++_
_:	ld	hl, (RandStartData)
	call	InsertHL		; call XXXXXX
_:	ld	a, 0EBh
	call	InsertA			; ex de, hl
	bit	need_push, (iy+myFlags)
	ret	z
	ld	a, 0E1h
	jp	InsertA			; pop hl
GetFunctionBC:
	ld	a, h
	cp	a, tGetKey
	jr	nz, +_
	ld	a, 0E5h
	bit	need_push, (iy+myFlags)
	call	nz, InsertA		; push hl
	ld	hl, _GetCSC
	call	InsertCallHL		; call _GetCSC
	ld	a, 0E1h
	bit	need_push, (iy+myFlags)
	call	nz, InsertA		; pop hl
	ld	a, 001h
	call	InsertA			; ld bc, *
	xor	a, a
	ld	hl, 05F0000h
	jp	InsertAHL		; ld bc, 0 \ ld e, a
_:	cp	a, trand
	jp	nz, InvalidTokenError
	ld	a, 0E5h
	bit	need_push, (iy+myFlags)
	call	nz, InsertA		; push hl
	ld	a, 0CDh
	call	InsertA			; call *
	call	InsertProgramPtrToDataOffset
	bit	has_already_rand, (iy+myFlags5)
	jr	nz, +_
	ld	hl, (programDataDataPtr)
	ld	(RandStartData), hl
	push	hl
	pop	de
	call	InsertHL		; call XXXXXX
	ld	hl, RandRoutine
	ld	bc, RandRoutineEnd - RandRoutine
	ldir
	ld	(programDataDataPtr), de
	set	has_already_rand, (iy+myFlags5)
	jr	++_
_:	ld	hl, (RandStartData)
	call	InsertHL		; call XXXXXX
_:	ld	a, 0E5h
	call	InsertA			; push hl
	ld	a, 0C1h
	call	InsertA			; pop bc
	bit	need_push, (iy+myFlags)
	ret	z
	ld	a, 0E1h
	jp	InsertA			; pop hl
    
CompareStrings:
	ld	a, (de)
	cp	a, (hl)
	inc	hl
	inc	de
	ret	nz
	cp	a, tEnter
	jr	nz, CompareStrings
	ret
    
SubError:
	ld	a, '-'
	jr	DispOperatorErrorString
AddError:
	ld	a, '+'
	jr	DispOperatorErrorString
DivError:
	ld	a, '/'
	jr	DispOperatorErrorString
MulError:
	ld	a, '*'
	jr	DispOperatorErrorString
    
GLETError:
	ld	a, (tempToken2)
	ld	c, a
	cp	a, tGE
	ld	a, 019h
	jr	z, DispOperatorErrorString
	ld	a, c
	cp	a, tLE
	ld	a, 017h
	jr	z, DispOperatorErrorString
	ld	a, c
	cp	a, tGT
	ld	a, '>'
	jr	z, DispOperatorErrorString
	ld	a, '<'
	jr	DispOperatorErrorString

NEQError:
	ld	a, (tempToken2)
	cp	a, tEq
	ld	a, '='
	jr	z, DispOperatorErrorString
	ld	a, 018h
	jr	DispOperatorErrorString
XORANDError:
	ld	a, (tempToken2)
	ld	c, a
	cp	a, tOr
	ld	a, '|'
	jr	z, DispOperatorErrorString
	ld	a, c
	cp	a, tXor
	ld	a, '^'
	jr	z, DispOperatorErrorString
	ld	a, '&'
	jr	DispOperatorErrorString
StoError:
	ld	a, 01Ch
	jr	DispOperatorErrorString
    
DispOperatorErrorString:
	push	af
	call	ClearScreen
	ld	hl, ErrorMessageStandard
	call	PrintString
	pop	af
	call	_PrintChar_ASM
	ld	a, '\''
	call	_PrintChar_ASM
	jr	ReturnToOS
    
ErrorSyntax:
	ld	hl, SyntaxErrorMessage
	jr	DispFinalString
MismatchError:
	ld	hl, MismatchErrorMessage
	jr	DispFinalString
NoProgramsError:
	ld	hl, NoProgramsMessage
	set	good_compilation, (iy+myFlags)
	jr	DispFinalString
InvalidTokenError:
	ld	hl, InvalidTokenMessage
	jr	DispFinalString
InvalidNameLength:
	ld	hl, InvalidNameLengthMessage
	jr	DispFinalString
SameNameError:
	ld	hl, SameNameMessage
	jr	DispFinalString
UnknownError:
	ld	hl, UnknownMessage
	jr	DispFinalString
WrongSpriteDataError:
	ld	hl, WrongSpriteDataMessgae
	jr	DispFinalString
ImplementError:
	ld	hl, ImplementMessage
	jr	DispFinalString
EndError:
	ld	hl, EndErrorMessage
	jr	DispFinalString
ErrorTooLargeLoop:
	ld	hl, TooLargeLoopMessage
	jr	DispFinalString
LabelError:
	ld	hl, LabelErrorMessage
    
DispFinalString:
	push	hl
	call	ClearScreen
	pop	hl
	call	PrintString
ReturnToOS:
	bit	good_compilation, (iy+myFlags)
	jr	nz, SkipDisplayLineNumber
	ld	a, 21
	ld	(TextYPos_ASM), a
	ld	hl, 1
	ld	(TextXPos_ASM), hl
	ld	hl, LineNumber
	call	PrintString
	ld	hl, (curPC)
	ld	de, (begPC_)
	or	a, a
	sbc	hl, de
	push	hl
	pop	bc
	sbc	hl, hl
	inc	hl
	ex	de, hl
GetAmountOfLines:
	ld	a, b
	or	a, c
	jr	z, +_
	ld	a, tEnter
	cpir
	jr	nz, +_
	inc	de
	jr	GetAmountOfLines
_:	ex	de, hl
	xor	a, a
	ld	de, OP3+10
	ld	(de), a
	dec	de
	ld	b, 8
	ld	a, 10
_:	call	_DivHLByA
	add	a, t0
	ld	(de), a
	dec	de
	add	hl, de
	or	a, a
	sbc	hl, de
	jr	z, +_
	djnz	-_
_:	ex	de, hl
	inc	hl
	call	PrintString
SkipDisplayLineNumber:
	ld	a, 230
	ld	(TextYPos_ASM), a
	ld	hl, 85
	ld	(TextXPos_ASM), hl
	ld	hl, PressKey
	call	PrintString
_:	call	_GetCSC
	or	a, a
	jr	z, -_
StopProgram:
	ld	sp, (backupSP)
	ld	hl, (backupCurPC)
	ld	(curPC), hl
	ld	hl, (backupEndPC)
	ld	(endPC), hl
	ld	a, lcdBpp16
	ld	(mpLcdCtrl), a
	call	_ClrWindow
	jp	_DrawStatusBar
    
ClearScreen:
	ld	hl, vRAM+(320*12)
	ld	(hl), 255
	push	hl
	pop	de
	inc	de
	ld	bc, 320*228-1
	ldir
	ld	hl, 1
	ld	(TextXPos_ASM), hl
	ld	a, 12
	ld	(TextYPos_ASM), a
	xor	a, a
	ld	(color), a
	ret
    
PrintString:
	ld	a, (hl)
	inc	hl
	or	a, a
	call	nz, _PrintChar_ASM
	jr	nz, PrintString
	ret
    
PreScanProgram:
	ld	hl, (curPC)
	ld	bc, (programSize)
ScanLoop:
	ld	a, tEnter
	cpir
	ret	nz
	ld	a, (hl)
	cp	a, tDet
	jr	z, ScanFoundDet
	cp	a, tPause
	jr	z, ScanFoundPause
	cp	a, tInput
	jr	z, ScanFoundInput
	jr	ScanLoop
ScanFoundInput:
	ld	a, (amountOfInput)
	inc	a
	ld	(amountOfInput), a
	jr	ScanLoop
ScanFoundPause:
	ld	a, (amountOfPause)
	inc	a
	ld	(amountOfPause), a
	jr	ScanLoop
ScanFoundDet:
	inc	hl
	ld	a, (hl)
	sub	a, t0
	jr	c, ScanLoop
	cp	a, t9-t0+1
	jr	nc, ScanLoop
	ld	de, 0
	ld	e, a
	inc	hl
	dec	bc
	ld	a, b
	or	a, c
	jr	z, FoundRightCFunction
	ld	a, (hl)
	cp	a, tComma
	jr	z, FoundRightCFunction
	cp	a, tEnter
	jr	z, FoundRightCFunction
	sub	a, t0
	jr	c, FoundRightCFunction
	cp	a, t9-t0+1
	jr	nc, FoundRightCFunction
	push	hl
	ex	de, hl
	add	hl, hl
	push	hl
	pop	de
	add	hl, hl
	add	hl, hl
	add	hl, de
	ld	de, 0
	ld	e, a
	add	hl, de
	ex	de, hl
	pop	hl
FoundRightCFunction:
	push	hl
	ex	de, hl
	ld	de, 77
	or	a, a
	sbc	hl, de
	jr	nc,  WrongCFunction
	add	hl, de
	ex	de, hl
	ld	hl, usedCroutines
	add	hl, de
	ld	a, (hl)
	or	a, a
	jr	nz, FunctionAlreadyInProgram
AddCFunctionToProgram:
	ld	a, (amountOfCRoutines)
	inc	a
	ld	(hl), a
	ld	(amountOfCRoutines), a
	ld	a, 0C3h
	push	de
	pop	hl
	add	hl, hl
	add	hl, de
	call	InsertAHL		; jp *
FunctionAlreadyInProgram:
WrongCFunction:
	pop	hl
	jp	ScanLoop

GetSpriteData:
	or	a, a
	sbc	hl, de
	ret	z
	add	hl, de
	ld	a, (de)
	sub	a, t0
	jp	c, WrongSpriteDataError
	cp	a, tA-t0
	jr	c, +_
	sub	a, tA-t0
	cp	a, tG-tA
	jp	nc, WrongSpriteDataError
	add	a, 10
_:	add	a, a
	add	a, a
	add	a, a
	add	a, a
	ld	ixh, a
	inc	de
	or	a, a
	sbc	hl, de
	jp	z, ErrorSyntax
	add	hl, de
	ld	a, (de)
	sub	a, t0
	jp	c, WrongSpriteDataError
	cp	a, tA-t0
	jr	c, +_
	sub	a, tA-t0
	cp	a, tG-tA
	jp	nc, WrongSpriteDataError
	add	a, 10
_:	or	a, ixh
	ld	(bc), a
	inc	bc
	inc	de
	jr	GetSpriteData