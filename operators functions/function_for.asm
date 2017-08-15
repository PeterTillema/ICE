functionFor:
	ld	hl, amountOfEnds
	inc	(hl)
	ld	a, 1
	ld	(openedParensF), a
	ld	(iy+fFunction2), 0
	call	_IncFetch
	sub	tA
	jp	c, functionForSmall
	cp	ttheta+1-tA
	jp	nc, functionForSmall
;    loop variable
	ld	b, a
	add	a, a
	add	a, b
	ld	(ForVariable1), a
	ld	l, a
	push	hl
	call	_IncFetch
_:	jp	c, ErrorSyntax
	cp	tComma
	jp	nz, ErrorSyntax
	call	_IncFetch
;    loop start point
	ld	hl, (programPtr)
	push	hl
	ld	hl, tempArg1
	ld	(programPtr), hl
	call	ParseExpression
	bit	triggered_a_comma, (iy+fExpression3)
	res	triggered_a_comma, (iy+fExpression3)
	jp	z, ErrorSyntax
ForVariable1 = $+3
	ld	hl, 0002FDDh
	call	InsertHL                                        ;    ld (ix+*), hl
	call	_IncFetch
	jr	c, -_
	ld	hl, (programPtr)
	ld	de, tempArg1
	or	a, a
	sbc	hl, de
	ex	de, hl
	pop	hl
	ld	(programPtr), hl
	push	de
;    loop end point
	call	ParseExpression
	bit	output_is_number, (iy+fExpression1)
	jr	nz, ForEndPointIsNumber
ForEndPointIsExpression:
	ld	a, 022h
	call	InsertA                                        ;    ld (******), hl
	ld	hl, (programPtr)
	ld	(ForEndPointExpression), hl
	call	InsertHL                                        ;    ld (RANDOM), hl
	jr	ForGetStep
ForEndPointIsNumber:
	set	end_point_is_number, (iy+fFunction2)
	ld	hl, (programPtr)
	dec	hl
	dec	hl
	dec	hl
	ld	de, (hl)
	dec	hl
	ld	(programPtr), hl
	ld	(ForFixedEndPoint), de
ForGetStep:
	bit	triggered_a_comma, (iy+fExpression3)
	res	triggered_a_comma, (iy+fExpression3)
	jr	nz, +_
;    loop step
	set	for_step_is_number, (iy+fFunction2)
	ld	hl, 1
	push	hl
	jr	ForStart
_:	call	_IncFetch
	jp	c, ErrorSyntax
	cp	tChs
	jr	nz, +_
	set	negative_for_step, (iy+fFunction2)
	call	_IncFetch
	jp	c, ErrorSyntax
_:	cp	a, tA
	jr	c, ForGetStepNumber
	cp	ttheta+1
	jp	nc, ForGetStepNumber
ForGetStepVariable:
	res	for_step_is_number, (iy+fFunction2)
	call	_CurFetch
	sub	a, tA
	ld	c, a
	call	InsertHIXC                                        ;    ld hl, (ix+*)
	call	_NxtFetch
	jp	c, ErrorSyntax
	cp	tEnter
	jp	nz, ErrorSyntax
	ld	a, 022h
	call	InsertA                                        ;    ld (*), hl
	ld	hl, (programPtr)
	push	hl
	call	InsertHL                                    ;    ld (RANDOM), hl
	jr	ForStart
ForGetStepNumber:
	set	for_step_is_number, (iy+fFunction2)
	call	ParseExpression
	bit	triggered_a_comma, (iy+fExpression3)
	jr	nz, -_
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
	push	hl
ForStart:
	pop	hl
	pop	de
	push	hl
	ld	hl, (programPtr)
	add	hl, de
	push	hl
	push	de
	pop	bc
	ld	de, (programPtr)
	ld	hl, tempArg1
	ldir
	ld	(programPtr), de
	bit	end_point_is_number, (iy+fFunction2)
	jr	nz, ForInsertEndPointNumber
ForInsertEndPointExpression:
	ld	a, 0B7h
	bit	negative_for_step, (iy+fFunction2)
	jr	nz, +_
	bit	end_point_is_number, (iy+fFunction2)
	jr	nz, +_
	ld	a, 037h
_:	ld	(ForSetCarryFlag), a
	ld	a, 011h
	call	InsertA                                        ;    ld de, *
	ld	hl, (programPtr)
	ld	de, UserMem-program
	add	hl, de
	ex	de, hl
ForEndPointExpression = $+1
	ld	hl, 0
	ld	(hl), de
	call	InsertHL                                        ;    ld de, RANDOM
	jr	ForSkip
ForInsertEndPointNumber:
	ld	a, 011h
ForFixedEndPoint = $+1
	ld	hl, 0
	bit	negative_for_step, (iy+fFunction2)
	jr	nz, +_
	inc	hl
_:	call	InsertAHL                                        ;    ld de, *
ForSkip:
ForSetCarryFlag = $+1
	ld	hl, 052EDB7h
	call	InsertHL                                        ;    or a \ sbc hl, de
	ld	a, 0D2h
	bit	negative_for_step, (iy+fFunction2)
	jr	z, +_
	add	a, 8
_:	call	InsertA                                        ;    jp [n]c, *
	ld	hl, (programPtr)
	push	hl
	call	InsertHL                                    ;    jp [n]c, RANDOM
	ld	b, (iy+fFunction2)
	push	bc
	call	ParseProgramUntilEnd
	pop	bc
	ld	(iy+fFunction2), b
	cp	a, tElse
	jp	z, ErrorSyntax
	ld	ix, 0
	add	ix, sp
	ld	a, (ix+9)
	ld	hl, 00027DDh
	call	_SetHLUToA
	call	InsertHL                                    ;    ld hl, (ix+*)
	pop	bc
	pop	de
	pop	hl
	push	de
	push	bc
	bit	for_step_is_number, (iy+fFunction2)
	jr	z, InsertVariableChange
InsertNumberChange:
	ex	de, hl
	ld	a, 1
	ld	(ExprOutput), a
	bit	negative_for_step, (iy+fFunction2)
	jr	z, $+8
	call	SubChainAnsNumber
	jr	$+6
	call	AddChainAnsNumber
	jr	InsertStop
InsertVariableChange:
	ex	de, hl
	ld	a, 011h
	call	InsertA                                        ;    ld de, *
	ld	hl, (programPtr)
	ld	bc, UserMem-program
	add	hl, bc
	ex	de, hl
	ld	(hl), de
	call	InsertHL                                        ;    ld de, RANDOM
	ld	a, 019h
	ld	hl, 052EDB7h
	bit	negative_for_step, (iy+fFunction2)
	call	nz, InsertHL                                    ;    or a \ sbc hl, de
	call	z, InsertA                                        ;    add hl, de
InsertStop:
	pop	bc
	pop	de
	pop	hl
	ld	a, l
	ld	hl, 0002FDDh
	call	_SetHLUToA
	call	InsertHL                                                    ;    ld (ix+*), hl
	or	a, a
	sbc	hl, de
	ld	a, h
	or	a, a
	jr	nz, ForBigLoop
	ld	a, l
	cpl
	dec	a
	cp	a, 010000000b
	jr	nc, ForSmallLoop
ForBigLoop:
	ld	a, 0C3h
	bit	negative_for_step, (iy+fFunction2)
	jr	z, $+4
	ld	a, 0D2h
	ld	hl, UserMem-program
	add	hl, de
	call	InsertAHL                                                    ;    jp [nc], ******
	jr	ForLoopInsert
ForSmallLoop:
	ld	ixl, a
	ld	a, 018h
	bit	negative_for_step, (iy+fFunction2)
	jr	z, $+4
	ld	a, 030h
	call	InsertA                                                    ;    jr [nc], **
	ld	a, ixl
	call	InsertA                                                    ;    jr [nc], **
	ld	hl, (programPtr)
ForLoopInsert:
	ld	de, UserMem-program
	add	hl, de
	ex	de, hl
	push	bc
	pop	hl
	ld	(hl), de
	ret

functionForSmall:
	call	ParseExpression
	bit	output_is_number, (iy+fExpression1)
	jr	z, +_
	ld	hl, (programPtr)
	dec	hl
	dec	hl
	ld	(programPtr), hl
	dec	hl
	dec	hl
	ld	(hl), 06h                                                     ;    ld b, *
	jr	++_
_:	ld	a, 045h
	call	InsertA                                                    ;    ld b, l
_:	ld	hl, (programPtr)
	push	hl
	ld	a, 0C5h
	call	InsertA                                                ;    push bc
	call	ParseProgramUntilEnd
	ld	a, 0C1h
	call	InsertA                                                ;    pop bc
	pop	de
	ld	hl, (programPtr)
	or	a, a
	sbc	hl, de
	ld	a, h
	or	a, a
_:	jp	nz, ErrorTooLargeLoop
	ld	a, l
	cpl
	dec	a
	cp	a, 010000000b
	jp	c, -_
	ld	b, a
	ld	a, 010h
	call	InsertA                                                    ;    djnz *
	ld	a, b
	jp	InsertA                                                        ;    djnz *