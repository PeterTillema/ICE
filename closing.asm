	ld	hl, openedParensE
	ld	b, a
	ld	a, (hl)
	or	a, a
	jr	z, MismatchedParens
	ld	a, b
	cp	a, tComma
	jr	z, +_
	dec	(hl)
_:	ld	hl, (stackPtr)
	ld	bc, stack
	or	a, a
	sbc	hl, bc
	jr	z, StopMovingForArgument2
	add	hl, bc
	dec	hl
	dec	hl
	dec	hl
	dec	hl
	ld	a, (hl)
	cp	a, typeFunction
	jr	z, StopMovingForArgument
	cp	a, typeOperator
	jr	nz, +_
	inc	hl
	ld	a, (hl)
	dec	hl
	or	a, a
	jr	z, StopMovingForArgument
_:	ld	(stackPtr), hl
	ld	de, (outputPtr)
	ldi
	ldi
	ldi
	ldi
	ld	(outputPtr), de
	jr	--_
_:	add	hl, bc
StopMovingForArgument:
	ld	a, (tempToken)
	cp	a, tComma
	jr	z, StopMovingForArgument2
	ld	(stackPtr), hl
	inc	hl
	ld	a, (hl)
	cp	a, tLParen
	jr	z, StopMovingForArgument2
	dec	hl
	ld	de, (outputPtr)
	ldi
	ldi
	ldi
	ldi
	ld	(outputPtr), de
StopMovingForArgument2:
	call	_IncFetch
	jp	MainLoop
MismatchedParens:
	ld	a, (openedParensF)
	or	a, a
	jp	z, MismatchError
	ld	a, (tempToken)
	cp	a, tComma
	jr	nz, +_
	set	triggered_a_comma, (iy+fExpression3)
_:	jp	StopParsing