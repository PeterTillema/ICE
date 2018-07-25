.assume adl = 1
segment data
.def _TostringData

_TostringData:
	ld	de, 0
	ld	bc, 0
DivideLoop:
	ld	a, 10
	call	0021D90h		; _DivHLByA
	dec	de
	inc	bc
	add	hl, de
	add	a, '0'
	ld	(de), a
	sbc	hl, de
	jr	nz, DivideLoop
	ex	de, hl
	ret
	
TempStringData:
	.db	0, 0, 0, 0, 0, 0, 0, 0
