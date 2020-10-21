assume adl = 1
public _RandData

_RandData:
	xor	a, a
	ld	(0), hl			; __state
	ld	hl, 0			; __state + 3
	ld	(hl), a
	ld	b, 12
__setstateloop:
	inc	hl
	ld	(hl), b
	djnz	__setstateloop
	ret
	ld	iy, 0			; __state
	ld	hl, (iy+0*4+0)
	push	hl
	ld	hl, (iy+0*4+2)
	push	hl
	lea	hl, iy+1*4
	lea	de, iy+0*4
	ld	bc, 3*4
	ldir
	pop	bc
	pop	de
	ld	h, d
	ld	l, e
	add	hl, hl
	add	hl, hl
	add	hl, hl
	ld	a, b
	xor	a, h
	ld	h, a
	xor	a, (iy+3*4+2)
	ld	(iy+3*4+3), a
	ld	b, a
	ld	a, c
	xor	a, l
	ld	l, a
	xor	a, (iy+3*4+1)
	ld	(iy+3*4+2), a
	xor	a, a
	add.s	hl, hl
	adc	a, a
	add.s	hl, hl
	adc	a, a
	add.s	hl, hl
	adc	a, a
	xor	a, d
	xor	a, (iy+3*4+0)
	ld	(iy+3*4+1), a
	ld	a, e
	xor	a, h
	ld	(iy+3*4+0), a
	ld	hl, (iy+3*4)
	ld	a, b
	ld	de, (iy+2*4)
	ld	c, (iy+2*4+3)
	add	hl, de
	ret

__state:
	db	10h, 0Fh, 0Eh, 0Dh
	db	0Ch, 0Bh, 0Ah, 09h
	db	08h, 07h, 06h, 05h
	db	04h, 03h, 02h, 01h
