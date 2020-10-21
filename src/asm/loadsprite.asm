assume adl = 1
public _LoadspriteData

_LoadspriteData:
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
	ret
