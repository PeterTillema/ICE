ParseLine:
	call	_CurFetch
	cp	a, tEnter
	ret	z
	ld	hl, FunctionsSingle
	ld	bc, FunctionsSingleEnd - FunctionsSingle
	cpir
	jr	nz, ParseExpression2
	cp	a, tVarOut
	jr	z, +_
	cp	a, tii
	jr	z, +_
	bit	used_code, (iy+fProgram1)
	set	used_code, (iy+fProgram1)
	call	z, UpdateSpritePointers
_:	ld	a, (openedParensF)
	or	a, a
	jp	nz, FunctionError
	ld	(iy+fFunction1), a
	ld	(iy+fFunction2), a
	ld	b, 3
	mlt	bc
	ld	hl, FunctionsSingleStart
	add	hl, bc
	ld	hl, (hl)
	jp	(hl)
    
ParseExpression2:
	bit	used_code, (iy+fProgram1)
	set	used_code, (iy+fProgram1)
	call	z, UpdateSpritePointers
ParseExpression:
	ld	hl, stack
	ld	(stackPtr), hl
	ld	hl, output
	ld	(outputPtr), hl
	xor	a, a
	ld	(openedParensE), a
	ld	(iy+fExpression1), a
	ld	(iy+fExpression2), a
	ld	(iy+fExpression3), a
	call	_CurFetch
MainLoopResCarryFlag:
	or	a, a    
MainLoop:
	ld	(tempToken), a
	jp	c, StopParsing
	cp	a, t0
	jr	c, NotANumber
	cp	a, t9+1
	jr	nc, NotANumber
ANumber:
#include "number.asm"

NotANumber:
	res	prev_is_number, (iy+fExpression1)
	cp	a, tA
	jr	c, NotAVariable
	cp	a, ttheta+1
	jr	nc, NotAVariable
AVariable:
	ld	hl, (outputPtr)
	ld	(hl), typeVariable
	inc	hl
	sub	a, tA
	jr	InsertAndUpdatePointer
NotAVariable:
	ld	hl, operators_booleans
	ld	bc, 15
	cpir
	jr	nz, NotABoolean
ABoolean:
#include "operator.asm"

ReturnToLoop:
	call	_IncFetch
	jp	MainLoop
NotABoolean:
	cp	a, tComma
	jr	z, CloseArgument
	cp	a, tRParen
	jp	nz, NotACommaOrRParen
CloseArgument:
#include "closing.asm"

NotACommaOrRParen:
	cp	a, tLBrace
	jp	nz, NotAList
AList:
#include "list.asm"

NotAList:
	cp	a, tVarLst
	jr	nz, NotAnOSList
AnOSList:
	call	_IncFetch
	cp	a, 6
	jp	nc, InvalidTokenError
	ld	c, a
	ld	b, 3
	mlt	bc
	ld	hl, lists
	add	hl, bc
	ld	hl, (hl)
	ex	de, hl
	ld	hl, (outputPtr)
	ld	(hl), typeOSList
	inc	hl
	ld	(hl), de
	inc	hl
	inc	hl
	inc	hl
	ld	(outputPtr), hl
	call	_IncFetch
	cp	a, tLParen
	jp	nz, MainLoopResCarryFlag
	ld	hl, openedParensE
	inc	(hl)
	ld	hl, (stackPtr)
	ld	(hl), typeOperator
	inc	hl
	ld	(hl), 0
	inc	hl
	inc	hl
	inc	hl
	ld	(stackPtr), hl
	call	_IncFetch
	jp	MainLoop
NotAnOSList:
	cp	a, tString
	jr	nz, NotAString
AString:
	ld	hl, (outputPtr)
	ld	(hl), typeString
	inc	hl
	ld	de, (tempStringsPtr)
	ld	(hl), de
	inc	hl
	inc	hl
	inc	hl
	ld	(outputPtr), hl
StringLoop:
	call	_IncFetch
	jr	c, StringStop2
	cp	a, tEnter
	jr	z, StringStop2
	cp	a, tString
	jr	z, StringStop
	cp	a, tStore
	jr	z, StringStop
	call	_IsA2ByteTok
	jr	nz, +_
	inc	hl
	ld	(curPC), hl
	dec	hl
_:	push	de
	call	_Get_Tok_Strng
	pop	de
	ld	hl, OP3
	ldir
	jr	StringLoop
StringStop:
	cp	a, tEnter
	call	nz, _IncFetch
StringStop2:
	ex	de, hl
	ld	(hl), 0
	inc	hl
	ld	(tempStringsPtr), hl
	jp	MainLoop
NotAString:
	cp	a, tEnter
	jp	z, StopParsing
	ld	hl, FunctionsWithReturnValue
	ld	bc, FunctionsWithReturnValueEnd - FunctionsWithReturnValue
	cpir
	jp	nz, InvalidTokenError
	cp	a, tGetKey
	jr	z, AddFunctionToOutput
	cp	a, tSqr
	jr	z, AddFunctionToOutput
	cp	a, trand
	jp	nz, AddFunctionToStack
AddFunctionToOutput:
	ld	hl, (outputPtr)
	ld	e, typeReturnValue
	cp	a, tSqr
	jr	nz, +_
	ld	e, typeFunction
_:	ld	(hl), e
	inc	hl
	ld	(hl), a
	inc	hl
	inc	hl
	inc	hl
	ld	(outputPtr), hl
	cp	a, tGetKey
	jp	nz, ReturnToLoop
	call	_IncFetch
	jp	c, MainLoop
	cp	a, tLParen
	jp	nz, MainLoopResCarryFlag
	call	_IncFetch
_:	jp	c, ErrorSyntax
	cp	a, tEnter
	jp	z, ErrorSyntax
	sub	a, t0
	jr	c, -_
	cp	a, t9-t0+1
_:	jp	nc, ErrorSyntax
	ld	de, 0
	ld	e, a
	call	_IncFetch
	jr	c, AddGetKeyDirect
	cp	a, tEnter
	jr	z, AddGetKeyDirect
	cp	a, tRParen
	jr	z, +_
	cp	a, tStore
	jr	z, AddGetKeyDirect
	sub	a, t0
	jr	c, --_
	cp	a, t9-t0+1
	jr	nc, -_
	push	de
	pop	hl
	add	hl, hl
	add	hl, hl
	add	hl, de
	add	hl, hl
	ld	e, a
	add	hl, de
	ex	de, hl
	call	_IncFetch
	jr	c, AddGetKeyDirect
	cp	a, tEnter
	jr	z, AddGetKeyDirect
	cp	a, tStore
	jr	z, AddGetKeyDirect
	cp	a, tRParen
	jp	nz, ErrorSyntax
_:	call	_IncFetch
AddGetKeyDirect:
	ld	hl, (outputPtr)
	dec	hl
	dec	hl
	dec	hl
	ld	(hl), e
	jp	MainLoop
AddFunctionToStack:
	ld	hl, openedParensE
	inc	(hl)
	call	_IsA2ByteTok
	call	z, _IncFetch
	ld	b, a
	ld	hl, (stackPtr)
	ld	a, (tempToken)
	ld	(hl), typeFunction
	inc	hl
	ld	(hl), a
	inc	hl
	ld	(hl), b
	inc	hl
	inc	hl
	ld	(stackPtr), hl
	jp	ReturnToLoop
StopParsing:                                                                ;    move stack to output
	call	MoveStackEntryToOutput
	ld	hl, (outputPtr)
	ld	de, output
	or	a, a
	sbc	hl, de
	push	hl
	pop	bc			;    BC / 4 is amount of elements in the stack
	push	de
	pop	hl
	ld	a, OutputIsInHL
	ld	(ExprOutput), a
	ld	(ExprOutput2), a
	ld	a, b
	or	a, c
	cp	a, 4
	ret	c
	jp	z, ParseSingleArgument
Loop:
	xor	a, a
	ld	(iy+fExpression1), a
	ld	(iy+fExpression2), a
	sbc	hl, bc
	ld	de, output
	sbc	hl, de
	jp	z, ErrorSyntax
	add	hl, de
	add	hl, bc
	push	hl
	ld	hl, 12
	or	a, a
	sbc	hl, bc
	jr	nz, +_
	set	op_is_last_one, (iy+fExpression1)
_:	pop	hl
	ld	a, b
	or	a, c
	cp	a, 4
	jp	z, StopParseExpression
	ld	a, (hl)
	cp	a, typeOperator
	jr	z, ExpressOperator
	cp	a, typeFunction
	jr	z, ExpressFunction
	inc	hl
	inc	hl
	inc	hl
	inc	hl
	jr	Loop
ExpressFunction:
	inc	hl			;    function = a
	ld	a, (hl)
	dec	hl
	push	bc
	push	hl
	call	ExecuteFunction
	pop	de
	push	de
	pop	hl
	inc	hl
	inc	hl
	inc	hl
	inc	hl
	ld	a, (amountOfArguments)
	dec	a
	jr	z, ++_
	ld	b, a
_:	dec	de
	dec	de
	dec	de
	dec	de
	djnz	-_
_:	pop	bc
	push	de
	push	bc
	push	hl
	ldir
	pop	hl
	pop	bc
	pop	de
	ex	de, hl
	add	hl, bc
	or	a, a
	sbc	hl, de
	push	hl
	pop	bc			;    BC = BC+DE-HL
	ld	a, b
	or	a, c
	cp	a, 4
	jr	nz, +_
	bit	output_is_number, (iy+fExpression1)
	jp	z, StopParseExpression
	ld	hl, (ix-3)
	jp	ParseSingleArgument2
_:	ex	de, hl
	ld	a, (amountOfArguments)
	ld	b, a
_:	dec	hl
	dec	hl
	dec	hl
	dec	hl
	djnz	-_
	jr	AddChain
ExpressOperator:
	inc	hl
	ld	a, (hl)
	dec	hl
	push	bc
	push	hl
	pop	ix
	ld	de, (ix-3)
	ld	bc, (ix-7)
	call	ExecuteOperator
	ld	a, (ExprOutput2)
	ld	(ExprOutput), a
	lea	de, ix-4
	pop	bc
	ld	hl, 8
	add	hl, de
	push	de
	push	bc
	ldir
	pop	bc
	ld	hl, -12
	add	hl, bc
	add	hl, de
	or	a, a
	sbc	hl, de
	push	hl
	pop	bc
	pop	hl
	jr	nz, +_
	bit	output_is_number, (iy+fExpression1)
	jp	z, StopParseExpression
	ld	hl, (ix-7)
	jr	ParseSingleArgument2
_:	inc	bc
	inc	bc
	inc	bc
	inc	bc
	bit	output_is_number, (iy+fExpression1)
	jp	nz, Loop
AddChain:
	push	hl
	pop	ix
	ld	e, typeChainAns
	ld	a, (hl)
	cp	a, typeOperator
	jr	nc, ChainAns2
	or	a, (ix+4)
	cp	a, typeOperator
	jr	z, ChainAns2
	cp	a, typeFunction
	jr	nz, ChainPush2
	ld	a, (ix+5)
	cp	a, tNot
	jr	nz, ChainAns2
ChainPush2:
	push	hl
	call	InsertPushHLDE
	pop	hl
	ld	e, typeChainPush
ChainAns2:
	ld	(ix-4), e
	jp	Loop
    
StopParseExpression:
	ld	a, (openedParensF)
	or	a, a
	jp	nz, MaybeChangeDEToHL
	ret
    
ParseSingleArgument:
	ld	a, (hl)
	or	a, a
	jr	nz, ParseSingleNotNumber
	set	output_is_number, (iy+fExpression1)
	inc	hl
	ld	hl, (hl)
ParseSingleArgument2:
	ld	a, 021h
	jp	InsertAHL                                                            ;    ld hl, *
ParseSingleNotNumber:
	dec	a
	jr	nz, ParseSingleNotVariable
	inc	hl
	ld	c, (hl)
	jp	InsertHIXC
ParseSingleNotVariable:
	sub	a, 3
	jr	nz, ParseSingleNotFunction
	inc	hl
	ld	a, (hl)
	ld	b, OutputInHL
	res	need_push, (iy+fExpression1)
	jp	GetFunction
ParseSingleNotFunction:
	sub	a, 3
	jp	nz, ErrorSyntax
	set	output_is_string, (iy+fExpression1)
	push	hl
	ld	a, 021h
	call	InsertA			;    ld hl, *
	call	InsertProgramPtrToDataOffset
	ld	hl, (programDataDataPtr)
	call	InsertHL                                                        ;    ld hl, XXXXXXX
	pop	hl
	inc	hl
	ld	de, (hl)                                                                ;    hl points to string in string stack
	ld	hl, (hl)
	ld	bc, -1
	xor	a, a
	cpir
	sbc	hl, de
	push	hl
	pop	bc			;    bc = length of string
	ex	de, hl
	ld	de, (programDataDataPtr)
	push	de
	ldir
	ld	(programDataDataPtr), de
	pop	hl
	ret
    
MoveStackEntryToOutput:
	ld	hl, (stackPtr)
	ld	de, stack
	or	a, a
	sbc	hl, de
	ret	z
	add	hl, de
	dec	hl
	dec	hl
	dec	hl
	dec	hl
	ld	(stackPtr), hl
	ld	de, (outputPtr)
	ld	a, (hl)
	cp	a, typeFunction
	jr	nz, +_
	inc	hl
	ld	a, (hl)
	dec	hl
	cp	a, tLParen
	jr	z, MoveStackEntryToOutput
_:	ldi
	ldi
	ldi
	ldi
	ld	(outputPtr), de
	jr	MoveStackEntryToOutput