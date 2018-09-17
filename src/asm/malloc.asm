.assume adl = 1
segment data
.def _MallocData

_MallocData:
	ld	de, 0
	add	hl, de
	ld	(0), hl
	ld	bc, 0D13ED4h		; saveSScreen + 21945 - 260
	or	a, a
	sbc	hl, bc
	sbc	hl, hl
	ret	nc
	ex	de, hl
	ret