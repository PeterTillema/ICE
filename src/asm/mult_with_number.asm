.assume adl = 1
segment data
.def _MultWithNumber
.ref _SetRegHLToRegDE
.ref _SetRegDEToRegHL
.ref _ResetHL
.ref _reg

_MultWithNumber:
	ld	iy, 0
	add	iy, sp
	ld	hl, (iy+3)
	ld	de, (iy+3)
	ld	b, 0
	ld	a, 26
GetFirstBitSetLoop:
	dec	a
	add	hl, hl
	jr	nc, GetFirstBitSetLoop
GetAmountOfOpcodesNeededLoop:
	adc	a, b
	adc	hl, hl
	jr	nz, GetAmountOfOpcodesNeededLoop
	rl	b
	jr	nz, DunnoWhatThisDoes
	sub	a, 3
DunnoWhatThisDoes:
	cp	a, 10
	jr	c, DoInsertHLDE
	ld	hl, -256
	add	hl, de
	jr	c, MultiplyLarge
MultiplySmall:
	ld	a, (iy+9)
	or	a, a
	ld	a, 0EBh
	call	nz, InsertA
	xor	a, a
	ld	(_reg), a
	ld	(_reg + 10), a
	ld	hl, 0CD003Eh
	ld	h, e
	ld	de, 0000150h
	ex	de, hl
InsertDEHL:			; instead of "jp InsertDEHL"
	push	hl
	call	InsertHLbis
	pop	hl
	jr	InsertHL
MultiplyLarge:
	ld	a, (iy+9)
	or	a, a
	ld	a, 0EBh
	call	nz, InsertA
	ex	de, hl
	ld	a, 001h
	call	InsertAHL
	xor	a, a
	ld	(_reg), a
	ld	(_reg + 10), a
	ld	hl, 0000154h
	ld	a, 0CDh		; instead of "jp InsertCallHL"
	jr	InsertAHL
DoInsertHLDE:
	djnz	DontNeedPushHLPopDE
	ld	a, (iy+9)
	or	a, a
	push	af
	call	z, _SetRegHLToRegDE
	pop	af
	call	nz, _SetRegDEToRegHL
	ld	a, 0E5h
	sub	a, (iy+9)
	call	InsertA
	ld	a, 0D1h
	add	a, (iy+9)
	call	InsertA
	jr	DontInsertEXDEHL2
InsertAHL:
	call	InsertA
InsertHL:
	ex	de, hl
InsertHLbis:
	ld	hl, (iy+6)
	push	hl
	ld	hl, (hl)
	ld	(hl), de
	inc	hl
	inc	hl
	inc	hl
	ex	de, hl
	pop	hl
	ld	(hl), de
	ret
DontNeedPushHLPopDE:
	ld	a, (iy+9)
	or	a, a
	ld	a, 0EBh
	call	nz, InsertA
DontInsertEXDEHL2:
	ex	de, hl
	scf
GetFirstSetBit2Loop:
	adc	hl, hl
	jr	nc, GetFirstSetBit2Loop
InsertAddHLDE:
	or	a, a
	adc	hl, hl
	ret	z
	call	_ResetHL
	ld	a, 029h
	call	InsertA
	ld	a, 019h
	jr	nc, .cont
	call	InsertA
	call	_ResetHL
.cont:
	jr	InsertAddHLDE
InsertA:
	push	hl
	ld	hl, (iy+6)
	push	hl
	ld	bc, (hl)
	ld	(bc), a
	inc	bc
	pop	hl
	ld	(hl), bc
	pop	hl
	ret
