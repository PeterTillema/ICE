assume adl = 1
public _XorData

_XorData:				; Credits to Runer112
	ld	bc, -1
	add	hl, bc
	sbc	a, a
	ex	de, hl
	add	hl, bc
	sbc	a, c
	add	a, c
	sbc	hl, hl
	inc	hl
