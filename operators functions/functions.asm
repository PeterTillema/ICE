ExecuteFunction:
	push	hl
	ld	hl, FunctionsWithReturnValueArguments
	ld	bc, FunctionsWithReturnValueEnd - FunctionsWithReturnValueArguments
	cpir
	ld	b, 3
	mlt	bc
	ld	hl, FunctionsWithReturnValueStart
	add	hl, bc
	ld	hl, (hl)
	ld	(JumpFunction), hl
	pop	hl
JumpFunction = $+1
	jp	0
    
functionPrgm:
	call	MaybeInsertIYFlags
	ld	a, 021h
	call	InsertA			; ld hl, *
	call	InsertProgramPtrToDataOffset
	ld	hl, (programDataDataPtr)
	push	hl
	call	InsertHL		; ld hl, programname
	pop	hl
	ld	(hl), ProgObj
	call	GetProgramName
	inc	hl
	ld	(programDataDataPtr), hl
	ld	a, 0CDh
	ld	hl, _Mov9ToOP1
	call	InsertAHL		; call _Mov9ToOP1
	ld	a, 0FDh
	ld	hl, 0CE08CBh
	call	InsertAHL		; set 1, (iy+8)
	ld	a, 0CDh
	ld	hl, _ParseInp
	call	InsertAHL		; call _ParseInp
	ld	a, 0FDh
	ld	hl, 08E08CBh
	jp	InsertAHL		; res 1, (iy+8)
    

functionOutput:
	call	MaybeInsertIYFlags
	ld	a, 1
	ld	(openedParensF), a
	call	_IncFetch
	call	ParseExpression
	bit	triggered_a_comma, (iy+fExpression3)
	res	triggered_a_comma, (iy+fExpression3)
	jp	z, ErrorSyntax
	bit	output_is_number, (iy+fExpression1)
	jr	nz, OutputRowIsNumber
OutputRowIsVariable:
	ld	a, 07Dh
	call	InsertA			; ld a, l
	jr	+_
OutputRowIsNumber:
	ld	hl, (programPtr)
	dec	hl
	dec	hl
	ld	(programPtr), hl
	dec	hl
	dec	hl
	ld	(hl), 03Eh		; ld a, *
_:	ld	a, 032h
	ld	hl, curRow
	call	InsertAHL		; ld (curRow), a
	call	_IncFetch
	call	ParseExpression
	bit	triggered_a_comma, (iy+fExpression3)
	res	triggered_a_comma, (iy+fExpression3)
	jp	z, ErrorSyntax
	bit	output_is_number, (iy+fExpression1)
	jr	nz, OutputColumnIsNumber
OutputColumnIsVariable:
	ld	a, 07Dh
	call	InsertA			; ld a, l
	jr	+_
OutputColumnIsNumber:
	ld	hl, (programPtr)
	dec	hl
	dec	hl
	ld	(programPtr), hl
	dec	hl
	dec	hl
	ld	(hl), 03Eh		; ld a, *
_:	ld	a, 032h
	ld	hl, curCol
	call	InsertAHL		; ld (curCol), a
	or	a, 1			; rest zero flag
	call	DisplayNumberOrString
	jp	nz, ErrorSyntax
	ret
    
functionDisp:
	call	MaybeInsertIYFlags
	ld	a, 1
	ld	(openedParensF), a
	dec	a			; set zero flag
	call	DisplayNumberOrString
	ret	z
	jr	functionDisp
    
DisplayNumberOrString:
	push	af
	call	_IncFetch
	call	ParseExpression
	ld	de, (programPtr)
	ld	bc, 14
	bit	output_is_string, (iy+fExpression1)
	jr	nz, DispString
	ld	hl, DispNumberRoutine
	pop	af
	jr	z, +_
	ld	hl, DispNumberRoutine-6
	ld	c, 8
	jr	+_
DispString:
	ld	hl, DispStringRoutine
	dec	bc
	pop	af
	jr	z, +_
	ld	hl, DispStringRoutine+5
	ld	c, 8
_:	ldir
	ld	(programPtr), de
	bit	triggered_a_comma, (iy+fExpression3)
	ret
    
functionLbl:
	ld	ix, (labelPtr)
	ld	hl, (programPtr)
	ld	(ix), hl
	ld	hl, (curPC)
	inc	hl
	ld	(ix+3), hl
	lea	hl, ix+6
	ld	(labelPtr), hl
	ld	b, tLbl
InsertLblGotoToDebugSection:
	bit	debug_on, (iy+fAlways1)
	jr	z, +_
	ld	hl, 011003Eh
	ld	h, b
	call	InsertHL		; ld a, * \ ld de, *
	call	InsertProgramPtrToDataOffset
	ld	hl, (programDataDataPtr)
	call	InsertHL
	ld	ix, programDataDataPtr
_:	call	_IncFetch
	jr	c, +_
	cp	a, tEnter
	jr	z, +_
	bit	debug_on, (iy+fAlways1)
	jr	z, -_
	ld	hl, (ix)
	ld	(hl), a
	inc	hl
	ld	(ix), hl
	jr	-_
_:	bit	debug_on, (iy+fAlways1)
	ret	z
	ld	hl, (ix)
	ld	(hl), 0
	inc	hl
	ld	(ix), hl
_:	ld	a, 0CDh
	call	InsertA			; call *
	call	InsertProgramPtrToDataOffset
	bit	has_already_debug, (iy+fProgram2)
	set	has_already_debug, (iy+fProgram2)
	jr	nz, +_
	ld	hl, (programDataDataPtr)
	push	hl
	call	InsertHL
	pop	de
	ld	(debugCodePtr), de
	ld	hl, InsertDebugCode
	ld	bc, InsertDebugCodeEnd - InsertDebugCode
	ldir
	ld	(programDataDataPtr), de
	ret
_:	ld	hl, (debugCodePtr)
	jp	InsertHL
    
functionCall:
	ld	a, 0CDh
	jr	+_
functionGoto:
	ld	a, 0C3h
_:	ld	ix, (gotoPtr)
	ld	hl, (programPtr)
	ld	de, 1
	bit	debug_on, (iy+fAlways1)
	jr	z, +_
	ld	e, 4+4+2+1
_:	add	hl, de
	ld	(ix), hl
	ld	hl, (curPC)
	inc	hl
	ld	(ix+3), hl
	lea	hl, ix+6
	ld	(gotoPtr), hl
	push	af
	ld	b, tGoto
	call	InsertLblGotoToDebugSection
	pop	af
	jp	InsertAHL		; jp/call RANDOM
    
functionSkipLine:
	ld	hl, (endPC)
	ld	de, (curPC)
	or	a, a
	sbc	hl, de
	ret	c
	inc	hl
	push	hl
	pop	bc
	ex	de, hl
	ld	a, tEnter
	cpir
	dec	hl
	ld	(curPC), hl
	ret
    
functionReturn:
	call	_NxtFetch
	jr	nc, +_
	set	last_token_is_ret, (iy+fExpression1)
_:	cp	tIf
	jr	nz, +_
	call	_IncFetch
	call	_IncFetch
	call	ParseExpression
	ld	a, 019h
	call	InsertA			; add hl, de
	ld	a, 0B7h
	ld	hl, 0C052EDh
	jp	InsertAHL		; or a \ sbc hl, de \ ret nz
_:	ld	a, 0C9h
	jp	InsertA			; ret
        
functionRepeat:
	ld	hl, amountOfEnds
	inc	(hl)
	ld	hl, (programPtr)
	ld	de, UserMem - program
	add	hl, de
	push	hl
	ld	hl, (curPC)
	inc	hl
	push	hl
_:	call	_IncFetch
	jp	c, ErrorSyntax
	cp	tEnter
	jr	nz, -_
	call	ParseProgramUntilEnd
	cp	tElse
	jp	z, ErrorSyntax
	ld	de, (curPC)
	pop	hl
	ld	(curPC), hl
	push	de
	call	ParseExpression
	pop	de
	ld	(curPC), de
	bit	output_is_number, (iy+fExpression1)
	jp	nz, functionRepeatInfinite
	bit	ans_set_z_flag, (iy+fExpression1)
	jr	z, +_
	ld	hl, (programPtr)
	dec	hl
	dec	hl
	dec	hl
	ld	a, (hl)
	ld	ixh, a
	dec	hl
	dec	hl
	dec	hl
	dec	hl
	ld	(programPtr), hl
	jr	functionRepeatInsert
_:	bit	last_token_was_not, (iy+fExpression2)
	jr	z, InsertNormalRepeat2
	ld	hl, (programPtr)
	ld	de, -8
	add	hl, de
	ld	(programPtr), hl
InsertNormalRepeat2:
	ld	a, 019h
	ld	hl, 052EDB7h
	call	InsertAHL		; add hl, de \ or a \ sbc hl, de
functionRepeatInsert:
	ld	bc, UserMem - program
	add	hl, bc
	pop	de
	or	a, a
	sbc	hl, de
	ld	a, h
	or	a, a
	jr	nz, functionRepeatLarge
	ld	a, l
	cpl
	dec	a
	cp	%10000000
	jr	c, functionRepeatLarge
functionRepeatSmall:
	ld	b, a
	ld	a, 028h
	bit	ans_set_z_flag, (iy+fExpression1)
	jr	z, +_
	ld	a, ixh
_:	bit	last_token_was_not, (iy+fExpression2)
	jr	z, $+4
	xor	8
	call	InsertA			; jr [n]z, *
	ld	a, b
	jp	InsertA			; jr [n]z, *
functionRepeatLarge:
	ex	de, hl
	bit	ans_set_z_flag, (iy+fExpression1)
	jr	z, +_
	ld	a, ixh
	add	a, 0A2h
	jr	++_
_:	ld	a, 0CAh
_:	bit	last_token_was_not, (iy+fExpression2)
	jr	z, $+4
	xor	8
	jp	InsertAHL		; jp [n]z, XXXXXX
functionRepeatInfinite:
	ld	hl, (programPtr)
	dec	hl
	dec	hl
	dec	hl
	dec	hl
	ld	(programPtr), hl
	push	hl
	inc	hl
	ld	hl, (hl)
	add	hl, de
	or	a, a
	sbc	hl, de
	pop	hl
	ret	nz
	ld	de, UserMem - program
	add	hl, de
	pop	de
	or	a, a
	sbc	hl, de
	ld	a, h
	or	a, a
	jr	nz, functionRepeatInfiniteLarge
	ld	a, l
	cpl
	dec	a
	cp	%10000000
	jr	c, functionRepeatInfiniteLarge
functionRepeatInfiniteSmall:
	ld	b, a
	ld	a, 018h
	call	InsertA			; jr *
	ld	a, b
	jp	InsertA			; jr *
functionRepeatInfiniteLarge:
	ex	de, hl
	ld	a, 0C3h
	jp	InsertAHL		; jp XXXXXX

functionIf:
	ld	hl, amountOfEnds
	inc	(hl)
	call	_IncFetch
	call	ParseExpression
	bit	output_is_number, (iy+fExpression1)
	jp	nz, functionIfInfinite
	bit	ans_set_z_flag, (iy+fExpression1)
	jr	z, InsertNormalIf
	ld	hl, (programPtr)
	dec	hl
	dec	hl
	dec	hl
	ld	a, (hl)
	add	a, 0A2h
	dec	hl
	dec	hl
	dec	hl
	dec	hl
	ld	(programPtr), hl
	jr	InsertIf
InsertNormalIf:
	bit	last_token_was_not, (iy+fExpression2)
	jr	z, InsertNormalIf2
	ld	hl, (programPtr)
	ld	de, -8
	add	hl, de
	ld	(programPtr), hl
InsertNormalIf2:
	ld	a, 019h
	ld	hl, 052EDB7h
	call	InsertAHL		; add hl, de \ or a \ sbc hl, de
	ld	a, 0CAh
	bit	last_token_was_not, (iy+fExpression2)
	jr	z, InsertIf
	ld	a, 0C2h
InsertIf:
	call	InsertA			; jp z, ******
	ld	hl, (programPtr)
	push	hl
	call	InsertHL		; jp z, XXXXXX
	call	ParseProgramUntilEnd
	cp	tElse
	jr	nz, +_
	ld	hl, amountOfEnds
	inc	(hl)
	ld	a, 0C3h
	call	InsertA			; jp ******
	ld	de, (programPtr)
	call	InsertHL		; jp XXXXXX
	ld	bc, UserMem - program
	add	hl, bc
	push	hl
	pop	bc
	pop	hl
	ld	(hl), bc
	push	de
	call	ParseProgramUntilEnd
	cp	tElse
	jp	z, ErrorSyntax
_:	ld	hl, (programPtr)
	ld	de, UserMem - program
	add	hl, de
	ex	de, hl
	pop	hl
	ld	(hl), de
	ret
functionIfInfinite:
	ld	hl, (programPtr)
	dec	hl
	dec	hl
	dec	hl
	dec	hl
	ld	(programPtr), hl
	inc	hl
	ld	hl, (hl)
	add	hl, de
	or	a, a
	sbc	hl, de
	jr	z, functionIfInfiniteFalse
	call	ParseProgramUntilEnd
	cp	tElse
	ret	nz
	ld	hl, (programPtr)
	push	hl
	call	ParseProgramUntilEnd
	cp	tElse
	jp	z, ErrorSyntax
	pop	hl
	ld	(programPtr), hl
	ret
functionIfInfiniteFalse:
	ld	hl, (programPtr)
	push	hl
	call	ParseProgramUntilEnd
	pop	hl
	ld	(programPtr), hl
	cp	tElse
	ret	nz
	call	ParseProgramUntilEnd
	cp	tElse
	jp	z, ErrorSyntax
	ret
    
functionWhile:
	ld	hl, amountOfEnds
	inc	(hl)
	call	_IncFetch
	jp	c, ErrorSyntax
	cp	tEnter
	jp	z, ErrorSyntax
	ld	hl, (programPtr)
	push	hl
	call	ParseExpression
	bit	output_is_number, (iy+fExpression1)
	jr	nz, functionWhileInfinite
	bit	ans_set_z_flag, (iy+fExpression1)
	jr	z, +_
	ld	hl, (programPtr)
	dec	hl
	dec	hl
	dec	hl
	ld	a, (hl)
	add	a, 0A2h
	dec	hl
	dec	hl
	dec	hl
	dec	hl
	ld	(programPtr), hl
	jr	InsertNormalWhile
_:	bit	last_token_was_not, (iy+fExpression2)
	jr	z, InsertNormalWhile2
	ld	hl, (programPtr)
	ld	de, -8
	add	hl, de
	ld	(programPtr), hl
InsertNormalWhile2:
	ld	a, 019h
	ld	hl, 052EDB7h
	call	InsertAHL		; add hl, de \ or a \ sbc hl, de
	ld	a, 0CAh
InsertNormalWhile:
	bit	last_token_was_not, (iy+fExpression2)
	jr	z, $+4
	xor	8
	call	InsertA			; jp z, ******
	ld	hl, (programPtr)
	push	hl
	call	InsertHL		; jp z, ******
	call	ParseProgramUntilEnd
	cp	tElse
	jp	z, ErrorSyntax
	ld	a, 0C3h
	pop	de
	pop	hl
	ld	bc, UserMem - program
	add	hl, bc
	call	InsertAHL		; jp XXXXXX
	add	hl, bc
	ex	de, hl
	ld	(hl), de
	ret
functionWhileInfinite:
	ld	hl, (programPtr)
	dec	hl
	dec	hl
	dec	hl
	dec	hl
	ld	(programPtr), hl
	inc	hl
	ld	hl, (hl)
	add	hl, de
	or	a, a
	sbc	hl, de
	jr	z, +_
	call	ParseProgramUntilEnd
	cp	tElse
	jp	z, ErrorSyntax
	ld	a, 0C3h
	pop	hl
	ld	bc, UserMem - program
	add	hl, bc
	jp	InsertAHL		; jp XXXXXX
_:	call	ParseProgramUntilEnd
	cp	tElse
	jp	z, ErrorSyntax
	pop	hl
	ld	(programPtr), hl
	ret
    
functionClrHome:
	call	_NxtFetch
	jr	c, +_
	cp	tEnter
	jp	nz, ErrorSyntax
_:	ld	hl, _HomeUp
	call	InsertCallHL		; call _HomeUp
	ld	hl, _ClrLCDFull
	jp	InsertAHL		; call _ClrLCDFull
    
functionPause:
	call	_IncFetch
	jr	c, ++_
	cp	tEnter
	jr	z, ++_
	call	ParseExpression
	ld	a, (amountOfPause)
	dec	a
	jp	z, functionPauseOnce
	bit	has_already_pause, (iy+fProgram1)
	jr	nz, AddPause
	ld	a, 0CDh
	call	InsertA			; call *
	call	InsertProgramPtrToDataOffset
	ld	hl, (programDataDataPtr)
	ld	(PauseStartData), hl
	push	hl
	pop	de
	bit	output_is_number, (iy+fExpression1)
	jr	z, +_
	push	hl
	ld	hl, (programPtr)
	dec	hl
	dec	hl
	dec	hl
	dec	hl
	ld	bc, (hl)
	dec	bc
	ld	(hl), bc
	pop	hl
	inc	hl
_:	call	InsertHL		; call *
	ld	hl, PauseRoutine
	ld	bc, PauseRoutineEnd - PauseRoutine
	ldir
	ld	(programDataDataPtr), de
	set	has_already_pause, (iy+fProgram1)
	ret
_:	ld	hl, _GetCSC
	call	InsertCallHL		; call _GetCSC
	ld	a, 0FEh
	ld	hl, 0F82009h
	jp	InsertAHL		; cp tEnter \ jr nz, $-9
AddPause:
	ld	a, 0CDh
	call	InsertA			; call ******
	call	InsertProgramPtrToDataOffset
	ld	hl, (PauseStartData)
	bit	output_is_number, (iy+fExpression1)
	jr	z, +_
	push	hl
	ld	hl, (programPtr)
	dec	hl
	dec	hl
	dec	hl
	dec	hl
	ld	de, (hl)
	dec	de
	ld	(hl), de
	pop	hl
	inc	hl
_:	jp	InsertHL		; call XXXXXX
functionPauseOnce:
	ld	de, (programPtr)
	ld	hl, PauseRoutine
	ld	bc, PauseRoutineEnd - PauseRoutine
	bit	output_is_number, (iy+fExpression1)
	jr	z, +_
	ld	ix, (programPtr)
	ld	de, (ix-3)
	dec	de
	ld	(ix-3), de
	lea	de, ix
	inc	hl
	dec	bc
_:	ldir
	dec	de
	ld	(programPtr), de
	ret
    
functionInput:
	call	MaybeInsertIYFlags
	call	_IncFetch
	cp	tA
	jp	c, ErrorSyntax
	cp	ttheta+1
	jp	nc, ErrorSyntax
	call	_NxtFetch
	jr	c, +_
	cp	tEnter
	jp	nz, ErrorSyntax
_:	ld	a, (amountOfInput)
	dec	a
	jr	z, functionInputOnce
	ld	a, 03Eh
	call	InsertA			; ld a, **
	call	_CurFetch
	sub	tA
	ld	b, a
	add	a, a
	add	a, b
	call	InsertA			; ld a, XX
	ld	a, 032h
	call	InsertA			; ld (******), a
	call	InsertProgramPtrToDataOffset
	bit	has_already_input, (iy+fProgram1)
	jr	nz, AddPointerToInput
	ld	hl, (programDataDataPtr)
	ld	(InputStartData), hl
	ld	bc, InputOffset-InputRoutine
	add	hl, bc
	call	InsertHL		; ld (XXXXXX), a
	ld	a, 0CDh
	call	InsertA			; call ******
	call	InsertProgramPtrToDataOffset
	ld	hl, (programDataDataPtr)
	push	hl
	pop	de
	call	InsertHL		; call XXXXXX
_:	ld	hl, InputRoutine
	ld	bc, InputRoutineEnd-InputRoutine
	ldir
	ld	(programDataDataPtr), de
	set	has_already_input, (iy+fProgram1)
	ret
AddPointerToInput:
	ld	hl, (InputStartData)
	push	hl
	ld	bc, InputOffset-InputRoutine
	add	hl, bc
	call	InsertHL		; ld (XXXXXX), a
	ld	a, 0CDh
	call	InsertA			; call ******
	call	InsertProgramPtrToDataOffset
	pop	hl
	jp	InsertHL		; call XXXXXX
functionInputOnce:
	call	_CurFetch
	sub	tA
	ld	b, a
	add	a, a
	add	a, b
	ld	(InputOffset), a
	ld	de, (programPtr)
	ld	hl, InputRoutine
	ld	bc, InputRoutineEnd - InputRoutine
	ldir
	push	de
	pop	ix
	ld	(ix-4), 0CDh
	ld	(programPtr), de
	ret
    
functionNot:
	ld	a, 1
	ld	(amountOfArguments), a
	set	last_token_was_not, (iy+fExpression2)
	push	hl
	pop	ix
	ld	a, (ix-4)
	or	a, a
	jr	z, NotNumber
	dec	a
	jr	z, NotVariable
	dec	a
	jr	z, NotChainPush
	dec	a
	jr	z, NotChainAns
	dec	a
	jr	z, NotFunction
	jp	ErrorSyntax
NotNumber:
	set	output_is_number, (iy+fExpression1)
	ld	hl, (ix-3)
	add	hl, de
	or	a, a
	sbc	hl, de
	ld	hl, 0
	jr	nz, +_
	inc	hl
_:	ld	(ix-3), hl
	ret
NotVariable:
	ld	c, (ix-3)
	ld	b, 3
	mlt	bc
	ld	a, c
	ld	hl, 00027DDh
	call	_SetHLUToA
	call	InsertHL		; ld hl, (ix+*)
	jr	NotChainAns
NotChainPush:
	jp	UnknownError
NotChainAns:
	ld	a, 011h
	call	InsertA			; ld de, *
	ld	a, 0FFh
	ld	de, 019FFFFh
	ld	hl, 02362EDh
	jp	InsertADEHL		; ld de, -1 \ add hl, de \ sbc hl, hl \ inc hl
NotFunction:
	ld	a, (ix-3)
	ld	b, OutputInHL
	call	GetFunction
	jr	NotChainAns
    
functionCE:
	inc	hl
	inc	hl
	ld	a, (hl)
	cp	tRemainder
	jp	nz, ErrorSyntax
	ld	de, -10
	add	hl, de
        
functionRemainder:
	ld	a, 2
	ld	(amountOfArguments), a
	ld	a, (hl)
	ld	ixh, a
	inc	hl
	push	hl
	ld	bc, (hl)
	inc	hl
	inc	hl
	inc	hl
	ld	a, (hl)
	ld	ixl, a
	inc	hl
	ld	de, (hl)
	pop	hl
	ld	a, ixh
	or	a, a
	jr	z, RemainderNumberXXX
	dec	a
	jp	z, RemainderVariableXXX
	dec	a
	jp	z, RemainderChainPushXXX
	dec	a
	jp	z, RemainderChainAnsXXX
	dec	a
	jp	z, RemainderFunctionXXX
	jp	ErrorSyntax
RemainderNumberXXX:
	ld	a, ixl
	or	a, a
	jr	z, RemainderNumberNumber
	dec	a
	jr	z, RemainderNumberVariable
	dec	a
	jr	z, RemainderNumberChainPush
	dec	a
	jr	z, RemainderNumberChainAns
	dec	a
	jr	z, RemainderNumberFunction
	jp	ErrorSyntax
RemainderNumberNumber:
	set	output_is_number, (iy+fExpression1)
	push	hl
	push	bc
	pop	hl
	push	de
	pop	bc
	call	__idvrmu
	ex	de, hl
	pop	hl
	ld	(hl), de
	ret
RemainderNumberVariable:
	ld	a, 021h
	push	bc
	pop	hl
	call	InsertAHL		; ld hl, *
	jp	RemainderChainAnsVariable
RemainderNumberChainPush:
	jp	UnknownError
RemainderNumberChainAns:
	ld	de, 021C1E5h
	push	bc
	pop	hl
	call	InsertDEHL		; push hl \ pop bc \ ld hl, *
	ld	hl, __idvrmu
	jp	InsertCallHL		; call __idvrmu
RemainderNumberFunction:
	ld	a, e
	push	bc
	ld	b, OutputInBC
	call	GetFunction
	pop	hl
	ld	a, 021h
	call	InsertAHL		; ld hl, *
	ld	hl, __idvrmu
	jp	InsertCallHL		; call __idvrmu
RemainderVariableXXX:
	ld	a, ixl
	or	a, a
	jr	z, RemainderVariableNumber
	dec	a
	jr	z, RemainderVariableVariable
	dec	a
	jr	z, RemainderVariableChainPush
	dec	a
	jr	z, RemainderVariableChainAns
	dec	a
	jr	z, RemainderVariableFunction
	jp	ErrorSyntax
RemainderVariableNumber:
	call	InsertHIXC		; ld hl, (ix+*)
	jr	RemainderChainAnsNumber
RemainderVariableVariable
	call	InsertHIXC		; ld hl, (ix+*)
	jr	RemainderChainAnsVariable
RemainderVariableChainPush:
	jp	UnknownError
RemainderVariableChainAns:
	ld	a, 0E5h
	call	InsertA			; push hl
	ld	a, 0C1h
	call	InsertA			; pop bc
_:	call	InsertHIXC		; ld hl, (ix+*)
	ld	hl, __idvrmu
	jp	InsertCallHL		; call __idvrmu
RemainderVariableFunction:
	ld	a, e
	ld	b, OutputInBC
	call	GetFunction
	jr	-_
RemainderChainPushXXX:
	ld	a, ixl
	cp	typeChainAns
	jp	nz, UnknownError
	ld	hl, 0E1C1E5h
	call	InsertAHL		; push hl \ pop bc \ pop hl
	jr	+_
RemainderChainAnsXXX:
	ld	a, ixl
	or	a, a
	jr	z, RemainderChainAnsNumber
	dec	a
	jr	z, RemainderChainAnsVariable
	dec	a
	jr	z, RemainderChainAnsChainPush
	dec	a
	jr	z, RemainderChainAnsChainAns
	dec	a
	jr	z, RemainderChainAnsFunction
	jp	ErrorSyntax
RemainderChainAnsNumber:
	ex	de, hl
	ld	a, 001h
	call	InsertAHL		; ld bc, *
_:	ld	hl, __idvrmu
	jp	InsertCallHL		; call __idvrmu
RemainderChainAnsVariable:
	call	InsertIXC		; ld bc, (ix+*)
	jr	-_
RemainderChainAnsChainPush:
	jp	UnknownError
RemainderChainAnsChainAns:
	jp	UnknownError
RemainderChainAnsFunction:
	ld	a, e
	ld	b, OutputInBC
	set	need_push, (iy+fExpression1)
	call	GetFunction
	jr	-_
RemainderFunctionXXX:
	ld	a, ixl
	or	a, a
	jr	z, RemainderFunctionNumber
	dec	a
	jr	z, RemainderFunctionVariable
	dec	a
	jr	z, RemainderFunctionChainPush
	dec	a
	jr	z, RemainderFunctionChainAns
	dec	a
	jr	z, RemainderFunctionFunction
	jp	ErrorSyntax
RemainderFunctionNumber:
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jr	RemainderChainAnsNumber
RemainderFunctionVariable:
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jr	RemainderChainAnsVariable
RemainderFunctionChainPush:
	jp	UnknownError
RemainderFunctionChainAns:
	ld	a, 0E5h
	call	InsertA			; push hl
	ld	a, 0C1h
	call	InsertA			; pop bc
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jr	-_
RemainderFunctionFunction:
	ld	a, e
	ld	b, OutputInBC
	call	GetFunction
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jr	-_
    
functionMean:
	set	use_mean_routine, (iy+fExpression2)
functionMax:
	ld	a, 030h
	jr	+_
functionMin:
	ld	a, 038h
_:	ld	(MaxMinMeanInsertSMC), a
	ld	a, 2
	ld	(amountOfArguments), a
	dec	hl
	dec	hl
	dec	hl
	ld	de, (hl)
	dec	hl
	ld	a, (hl)
	ld	ixl, a
	dec	hl
	dec	hl
	dec	hl
	ld	bc, (hl)
	dec	hl
	ld	a, (hl)
	inc	hl
	or	a, a
	jr	z, MaxMinMeanNumberXXX
	dec	a
	jp	z, MaxMinMeanVariableXXX
	dec	a
	jp	z, MaxMinMeanChainPushXXX
	dec	a
	jp	z, MaxMinMeanChainAnsXXX
	dec	a
	jp	z, MaxMinMeanFunctionXXX
	jp	ErrorSyntax
MaxMinMeanNumberXXX:
	ld	a, ixl
	or	a, a
	jr	z, MaxMinMeanNumberNumber
	dec	a
	jr	z, MaxMinMeanNumberVariable
	dec	a
	jr	z, MaxMinMeanNumberChainPush
	dec	a
	jr	z, MaxMinMeanNumberChainAns
	dec	a
	jr	z, MaxMinMeanNumberFunction
	jp	ErrorSyntax
MaxMinMeanNumberNumber:
	set	output_is_number, (iy+fExpression1)
	push	hl
	bit	use_mean_routine, (iy+fExpression2)
	jr	nz, ++_
	ld	a, (MaxMinMeanInsertSMC)
	push	bc
	pop	hl
	cp	030h
	jr	z, +_
	or	a, a
	sbc	hl, de
	add	hl, de
	jr	nc, $+3
	ex	de, hl
	jr	+++_
_:	or	a, a
	sbc	hl, de
	add	hl, de
	jr	c, $+3
	ex	de, hl
	jr	++_
_:	ex	de, hl
	ld	ix, 0
	add	ix, sp
	add	hl, bc
	push	hl
	rr	(ix-1)
	pop	de
	rr	d
	rr	e
_:	pop	hl
	ld	(hl), de
	ret
MaxMinMeanNumberVariable:
	push	bc
	pop	hl
	ex	de, hl
	push	hl
	pop	bc
	jr	MaxMinMeanVariableNumber
MaxMinMeanNumberChainPush:
	jp	UnknownError
MaxMinMeanNumberChainAns:
	push	bc
	pop	de
	jr	+_
MaxMinMeanNumberFunction:
	ld	a, e
	push	bc
	ld	b, OutputInHL
	call	GetFunction
	pop	de
_:	jp	MaxMinMeanChainAnsNumber
MaxMinMeanVariableXXX:
	ld	a, ixl
	or	a, a
	jr	z, MaxMinMeanVariableNumber
	dec	a
	jr	z, MaxMinMeanVariableVariable
	dec	a
	jr	z, MaxMinMeanVariableChainPush
	dec	a
	jr	z, MaxMinMeanVariableChainAns
	dec	a
	jr	z, MaxMinMeanVariableFunction
	jp	ErrorSyntax
MaxMinMeanVariableNumber:
	call	InsertHIXC		; ld hl, (ix+*)
	jr	MaxMinMeanChainAnsNumber
MaxMinMeanVariableVariable
	call	InsertHIXC		; ld hl, (ix+*)
	jp	MaxMinMeanChainAnsVariable
MaxMinMeanVariableChainPush:
	ld	hl, (programPtr)
	dec	hl
	ld	a, (hl)
	cp	0E5h
	jr	z, +_
	ld	(hl), 0E1h
	inc	hl
_:	ld	(programPtr), hl
	ld	e, c
	jp	MaxMinMeanChainAnsVariable
MaxMinMeanVariableChainAns:
_:	ld	e, c
	call	InsertIXE		; ld de, (ix+*)
	jr	MaxMinMeanInsert
MaxMinMeanVariableFunction:
	ld	a, e
	ld	b, OutputInHL
	call	GetFunction
	jr	-_
MaxMinMeanChainPushXXX:
	ld	a, ixl
	cp	typeChainAns
	jp	nz, UnknownError
	bit	use_mean_routine, (iy+fExpression2)
	jr	nz, +_
	ld	a, 0EBh
	call	InsertA			; ex de, hl
	ld	a, 0E1h
	call	InsertA			; pop hl
	jr	MaxMinMeanInsert
_:	ld	a, 0D1h
	call	InsertA			; pop de
	jr	MaxMinMeanInsert
MaxMinMeanChainAnsXXX:
	ld	a, ixl
	or	a, a
	jr	z, MaxMinMeanChainAnsNumber
	dec	a
	jr	z, MaxMinMeanChainAnsVariable
	dec	a
	jr	z, MaxMinMeanChainAnsChainPush
	dec	a
	jr	z, MaxMinMeanChainAnsChainAns
	dec	a
	jr	z, MaxMinMeanChainAnsFunction
	jp	ErrorSyntax
MaxMinMeanChainAnsNumber:
	ex	de, hl
	ld	a, 011h
	call	InsertAHL		; ld de, *
MaxMinMeanInsert:
	bit	use_mean_routine, (iy+fExpression2)
	jp	nz, MeanInsert
	ld	a, 0B7h
	ld	de, 01952EDh
MaxMinMeanInsertSMC = $+1
	ld	hl, 0EB0130h
	jp	InsertADEHL		; or a \ sbc hl, de \ add hl, de \ jr [n]c, $+3 \ add hl, de
MaxMinMeanChainAnsVariable:
	call	InsertIXE		; ld de, (ix+*)
	jr	MaxMinMeanInsert
MaxMinMeanChainAnsChainPush:
	jp	UnknownError
MaxMinMeanChainAnsChainAns:
	jp	UnknownError
MaxMinMeanChainAnsFunction:
	ld	a, e
	ld	b, OutputInDE
	set	need_push, (iy+fExpression1)
	call	GetFunction
_:	jr	MaxMinMeanInsert
MaxMinMeanFunctionXXX:
	ld	a, ixl
	or	a, a
	jr	z, MaxMinMeanFunctionNumber
	dec	a
	jr	z, MaxMinMeanFunctionVariable
	dec	a
	jr	z, MaxMinMeanFunctionChainPush
	dec	a
	jr	z, MaxMinMeanFunctionChainAns
	dec	a
	jr	z, MaxMinMeanFunctionFunction
	jp	ErrorSyntax
MaxMinMeanFunctionNumber:
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jr	MaxMinMeanChainAnsNumber
MaxMinMeanFunctionVariable:
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jr	MaxMinMeanChainAnsVariable
MaxMinMeanFunctionChainPush:
	jp	UnknownError
MaxMinMeanFunctionChainAns:
	bit	use_mean_routine, (iy+fExpression2)
	jr	nz, ++_
	ld	a, 0EBh
	call	InsertA			; ex de, hl
	ld	a, c
	ld	b, OutputInHL
_:	set	need_push, (iy+fExpression1)
	call	GetFunction
	jr	MaxMinMeanInsert
_:	ld	a, c
	ld	b, OutputInDE
	jr	--_
MaxMinMeanFunctionFunction:
	ld	a, e
	ld	b, OutputInDE
	call	GetFunction
	ld	a, c
	ld	b, OutputInHL
	set	need_push, (iy+fExpression1)
	call	GetFunction
	jp	MaxMinMeanInsert
MeanInsert:
	bit	has_already_mean, (iy+fProgram1)
	jr	nz, +_
	ld	a, 0CDh
	call	InsertA			; call *
	call	InsertProgramPtrToDataOffset
	ld	hl, (programDataDataPtr)
	ld	(MeanStartData), hl
	push	hl
	pop	de
	call	InsertHL		; call *
	ld	hl, MeanRoutine
	ld	bc, MeanRoutineEnd - MeanRoutine
	ldir
	ld	(programDataDataPtr), de
	set	has_already_mean, (iy+fProgram1)
	ret
_:	ld	a, 0CDh
	call	InsertA			; call ******
	call	InsertProgramPtrToDataOffset
	ld	hl, (MeanStartData)
	jp	InsertHL		; call XXXXXX
    
functionSqrt:
	ld	a, 1
	ld	(amountOfArguments), a
	set	last_token_was_not, (iy+fExpression2)
	push	hl
	pop	ix
	ld	a, (ix-4)
	or	a, a
	jr	z, SqrtNumber
	dec	a
	jr	z, SqrtVariable
	dec	a
	jr	z, SqrtChainPush
	dec	a
	jr	z, SqrtChainAns
	dec	a
	jr	z, SqrtFunction
	jp	ErrorSyntax
SqrtNumber:
	set	output_is_number, (iy+fExpression1)
	ld	hl, (ix-3)
	push	hl
	pop	bc
	call	__imulu
	ld	(ix-3), hl
	ret
SqrtVariable:
	ld	c, (ix-3)
	ld	b, 3
	mlt	bc
	ld	a, c
	ld	hl, 00027DDh
	call	_SetHLUToA
	call	InsertHL		; ld hl, (ix+*)
	jr	SqrtChainAns2
SqrtChainPush:
	jp	UnknownError
SqrtChainAns:
	call	MaybeChangeDEToHL
SqrtChainAns2:
	ld	de, 0CDC1E5h
	ld	hl, __imuls
	jp	InsertDEHL		; push hl \ pop bc \ call __imuls
SqrtFunction:
	ld	a, (ix-3)
	ld	b, OutputInHL
	call	GetFunction
	jr	SqrtChainAns2
    
functionC:
	ld	a, 1
	ld	(openedParensF), a
	call	_IncFetch
	call	ParseExpression
	bit	output_is_number, (iy+fExpression1)
	jp	z, ErrorSyntax
	ld	hl, (programPtr)
	dec	hl
	dec	hl
	dec	hl
	dec	hl
	ld	(programPtr), hl
	inc	hl
	ld	hl, (hl)
	ld	de, AMOUNT_OF_C_FUNCTIONS
	or	a, a
	sbc	hl, de
	jp	nc, ErrorSyntax
	add	hl, de
	ld	a, l
	or	a, a
	sbc	hl, hl
	ld	l, a
	ld	de, CFunctionsArguments
	add	hl, de
	ld	b, a
	ld	a, (hl)
	cp	a, 011100000b
	jp	z, ImplementError
	ld	(iy+fFunction1), a
	rlca
	rlca
	rlca
	and	a, 000000111b
	ld	c, a
	ld	a, b
	ld	b, 3
	mlt	bc
	ld	hl, CArguments
	add	hl, bc
	ld	hl, (hl)
	ld	(CFunctionArgsSMC), a
	jp	(hl)

functionCustom:
	call	_IncFetch
	sub	10
	jp	c, ErrorSyntax
	cp	AMOUNT_OF_CUSTOM_TOKENS + 1
	jp	nc, ErrorSyntax
	ld	c, a
	ld	b, 3
	mlt	bc
	ld	hl, functionCustomStart
	add	hl, bc
	ld	hl, (hl)
	jp	(hl)
    
functionExecHex:
	call	_IncFetch
	ret	c
	cp	a, tEnter
	ret	z
	cp	a, tString
	ret	z
	ld	hl, hexadecimals
	ld	bc, 16
	cpir
_:	jp	nz, ErrorSyntax
	ld	d, c
	call	_IncFetch
	jp	c, ErrorSyntax
	ld	hl, hexadecimals
	ld	bc, 16
	cpir
	jr	nz, -_
	ld	a, d
	add	a, a
	add	a, a
	add	a, a
	add	a, a
	add	a, c
	call	InsertA
	jr	functionExecHex
    
functionSetBASICVar:
	call	_IncFetch
	jp	c, ErrorSyntax
	cp	a, tA
	jp	c, ErrorSyntax
	cp	a, ttheta+1
	jp	nc, ErrorSyntax
	push	af
	call	_IncFetch
	jp	c, ErrorSyntax
	cp	a, tComma
	jp	nz, ErrorSyntax
	call	_IncFetch
	jp	c, ErrorSyntax
	call	ParseExpression
	ld	a, 006h
	call	InsertA			; ld b, *
	pop	af
	call	InsertA
	ld	a, 0CDh
	call	InsertA
	call	InsertProgramPtrToDataOffset
	bit	has_already_setvar, (iy+fProgram2)
	set	has_already_setvar, (iy+fProgram2)
	jr	z, +_
	ld	hl, (SetBASICVarPtr)
	jp	InsertHL
_:	ld	hl, (programDataDataPtr)
	ld	(SetBASICVarPtr), hl
	push	hl
	call	InsertHL
	pop	de
	ld	hl, StoBASICVar
	ld	bc, StoBASICVarEnd-StoBASICVar
	ldir
	ld	(programDataDataPtr), de
	ret
    
functionGetBASICVar:
	ld	a, 006h
	call	InsertA			; ld b, *
	call	_IncFetch
	jp	c, ErrorSyntax
	cp	a, tA
	jp	c, ErrorSyntax
	cp	a, ttheta+1
	jp	nc, ErrorSyntax
	call	InsertA			; ld b, X
	ld	a, 0CDh
	call	InsertA			; call *
	call	InsertProgramPtrToDataOffset
	bit	has_already_getvar, (iy+fProgram2)
	set	has_already_getvar, (iy+fProgram2)
	jr	z, +_
	ld	hl, (GetBASICVarPtr)
	call	InsertHL
	jr	++_
_:	ld	hl, (programDataDataPtr)
	ld	(GetBASICVarPtr), hl
	push	hl
	call	InsertHL
	pop	de
	ld	hl, GetBASICVar
	ld	bc, GetBASICVarEnd-GetBASICVar
	ldir
	ld	(programDataDataPtr), de
_:	ld	a, 0DDh
	call	InsertA			; ld (ix+*), hl
	ld	a, 02Fh
	call	InsertA			; ld (ix+*), hl
	call	_IncFetch
	jp	c, ErrorSyntax
	cp	a, tComma
	jp	nz, ErrorSyntax
	call	_IncFetch
	jp	c, ErrorSyntax
	cp	a, tA
	jp	c, ErrorSyntax
	cp	a, ttheta+1
	jp	nc, ErrorSyntax
	sub	a, tA
	ld	b, a
	add	a, a
	add	a, b
	jp	InsertA			; ld (ix+*), hl
    
functionRoot:
	ld	a, 1
	ld	(amountOfArguments), a
	push	hl
	pop	ix
	ld	a, (ix-4)
	or	a, a
	jr	z, RootNumber
	dec	a
	jr	z, RootVariable
	dec	a
	jr	z, RootChainPush
	dec	a
	jr	z, RootChainAns
	dec	a
	jr	z, RootFunction
	jp	ErrorSyntax
RootNumber:
	set	output_is_number, (iy+fExpression1)
	ld	hl, (ix-3)
	push	iy
	call	RootRoutine
	pop	iy
	ld	(ix-3), hl
	ret
RootVariable:
	ld	c, (ix-3)
	ld	b, 3
	mlt	bc
	ld	a, c
	ld	hl, 00027DDh
	call	_SetHLUToA
	call	InsertHL		; ld hl, (ix+*)
	jr	RootChainAns
RootChainPush:
	jp	UnknownError
RootChainAns:
	bit	has_already_root, (iy+fProgram2)
	set	has_already_root, (iy+fProgram2)
	jr	nz, RootChainAnsRecallRoutine
RootChainAnsAddRoutine:
	set	modified_iy, (iy+fAlways1)
	ld	de, (programDataDataPtr)
	ld	(RootStartData), de
	ld	hl, RootRoutine
	ld	bc, RootRoutineEnd - RootRoutine
	ldir
	ld	(programDataDataPtr), de
RootChainAnsRecallRoutine:
	ld	a, 0CDh
	call	InsertA			; call *
	call	InsertProgramPtrToDataOffset
	ld	hl, (RootStartData)
	jp	InsertHL		; call *
RootFunction:
	ld	a, (ix-3)
	ld	b, OutputInHL
	call	GetFunction
	jr	RootChainAns
    
      
    
      
    
      
    
functionDefineSprite:
	bit	used_code, (iy+fProgram1)
	jp	nz, ErrorUsedCode
	ld	a, 1
	ld	(openedParensF), a
	call	InsertProgramPtrToDataOffset
	ld	hl, (programDataDataPtr)
	push	hl
	pop	de
	call	InsertHL
	ld	b, 2
_:	push	de
	push	bc
	call	_IncFetch
	call	ParseExpression
	bit	output_is_number, (iy+fExpression1)
	jp	z, ErrorSyntax
	bit	triggered_a_comma, (iy+fExpression3)
	jp	z, ErrorSyntax
	ld	hl, (programPtr)
	dec	hl
	dec	hl
	dec	hl
	dec	hl
	ld	(programPtr), hl
	inc	hl
	ld	a, (hl)
	pop	bc
	pop	de
	ld	(de), a
	inc	de
	djnz	-_
	ld	hl, (programPtr)
	push	hl
	ld	(programPtr), de
	call	_IncFetch
	cp	tString
	jp	nz, ErrorSyntax
	call	_NxtFetch
	jp	c, ErrorSyntax
	cp	tEnter
	jp	z, ErrorSyntax
	call	functionExecHex
	ld	hl, (programPtr)
	ld	(programDataDataPtr), hl
	pop	hl
	ld	(programPtr), hl
	ret
    
functionCompilePrgm:
	ld	hl, (OP1)
	push	hl
	ld	hl, (OP1+3)
	push	hl
	ld	hl, (OP1+6)
	push	hl
	ld	hl, OP1
	call	GetProgramName
	ld	a, ProgObj
	ld	(OP1), a
	call	_ChkFindSym
	jr	nc, +_
	ld	hl, OP1
	inc	(hl)
	call	_ChkFindSym
	jp	c, ErrorNotFound
_:	call	_ChkInRAM
	jr	nc, +_
	ex	de, hl
	ld	de, 9
	add	hl, de
	ld	e, (hl)
	add	hl, de
	inc	hl
	ex	de, hl
_:	ld	hl, (begPC)
	push	hl
	ld	hl, (curPC)
	push	hl
	ld	hl, (endPC)
	push	hl
	ex	de, hl
	ld	bc, 0
	ld	c, (hl)
	inc	hl
	ld	(curPC), hl
	ld	b, (hl)
	inc	hl
	ld	(begPC), hl
	add	hl, bc
	dec	hl
	ld	(endPC), hl
	call	PrintCompilingProgram
	ld	hl, AmountOfSubPrograms
	inc	(hl)
	call	CompileProgramFull
	pop	hl
	ld	(endPC), hl
	pop	hl
	ld	(curPC), hl
	pop	hl
	ld	(begPC), hl
	pop	hl
	ld	(OP1+6), hl
	pop	hl
	ld	(OP1+3), hl
	pop	hl
	ld	(OP1), hl
	jp	PrintCompilingProgram