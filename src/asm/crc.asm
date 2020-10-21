assume adl = 1
public _GetCRC

_GetCRC:
	pop	hl
	pop	de
	pop	bc
	push	bc
	push	de
	push	hl
	ld	hl, 000FFFFh
Read:	
	ld	a, b
	or	a, c
	ret	z
	push	bc
	ld	a, (de)
	inc	de
	xor	a, h
	ld	h, a
	ld	b, 8
.loop:
	add.s	hl, hl
	jr	nc, .next
	ld	a, h
	xor	a, 010h
	ld	h, a
	ld	a, l
	xor	a, 021h
	ld	l, a
.next:	
	djnz	.loop
	pop	bc
	dec	bc
	jr	Read
