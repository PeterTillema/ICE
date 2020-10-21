assume adl = 1
public _InputData

include 'ti84pce.inc'

_InputData:
	ld	de, ioPrompt
	ld	bc, 26
	ldir
	call	_ClrScrn
	call	_HomeUp
	xor	a, a
	ld	(curUnder), a
	ld	b, (iy+9)
	ld	c, (iy+28)
	res	6, (iy+28)
	set	7, (iy+9)
	push	bc
	call	_GetStringInput
	pop	bc
	res	4, b
	ld	(iy+9), b
	ld	(iy+28), c
	ld	hl, (editSym)
	call	_VarNameToOP1HL
	call	_ChkFindSym
	ld	a, (de)
	inc	de
	inc	de
	ld	b, a
	sbc	hl, hl
_loop:	push	bc
	add	hl, hl
	push	hl
	pop	bc
	add	hl, hl
	add	hl, hl
	add	hl, bc
	ld	a, (de)
	sub	a, 030h
	ld	bc, 0
	ld	c, a
	add	hl, bc
	inc	de
	pop	bc
	djnz	_loop
	ld	(ix+0), hl
	jp	_DeleteTempEditEqu
