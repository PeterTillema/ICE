assume adl = 1
public _LoadtilemapData

_LoadtilemapData:
	call	0020320h
	ld	a, 015h
	ld	(0D005F8h), a
	call	002050Ch
	ccf
	sbc	hl, hl
	ret	nc
	call	0021F98h
	call	c, 0021448h
	ld	hl, 0
	add	hl, de
	ld	c, (hl)
	inc	hl
	ld	b, (hl)
	dec	hl
	mlt	bc
	inc	bc
	inc	bc
	ld	de, 0
	push	de
	ld	a, 0
BuildTableLoop:
	ex	de, hl
	ld	(hl), de
	inc	hl
	inc	hl
	inc	hl
	ex	de, hl
	add	hl, bc
	dec	a
	jr	nz, BuildTableLoop
	pop	hl
	ret
