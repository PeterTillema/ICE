.assume adl = 1
segment data
.def _RandintData

_RandintData:
	ex	de, hl
	push	de
	or	a, a
	sbc	hl, de
	inc	hl
	push	hl
	call	0
	pop	bc
	call	0000144h
	pop	de
	add	hl, de
	ret
