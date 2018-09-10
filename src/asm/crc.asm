.assume adl = 1
segment data
.def _CRCData

_CRCData:
	pop	hl
	pop	de
	pop	bc
	push	bc
	push	de
	push	hl
	ld	hl, 000FFFFh
Read:	push	bc
	ld	a, (de)
	inc	de
	xor	a, h
	ld	h, a
	ld	b, 8
_:	add.s	hl, hl
	jr	nc, Next
	ld	a, h
	xor	a, 010h
	ld	h, a
	ld	a, l
	xor	a, 021h
	ld	l, a
Next:	djnz	-_
	pop	bc
	dec	bc
	ld	a, b
	or	a, c
	jr	nz, Read
	ret
