assume adl = 1
public _MeanData

_MeanData:
	add	hl, de
	ld	a, 1
_ShiftRight:
	ld	b, a
	ex	af, af'
	ex	de, hl
	scf
	sbc	hl, hl
	add	hl, sp
	ex	af, af'
__ShiftRightLoop:
	push	de
	rr	(hl)
	pop	de
	rr	d
	rr	e
	or	a, a
	djnz	__ShiftRightLoop
	ex	de, hl
	ret
