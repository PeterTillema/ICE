.assume adl = 1
segment data
.def _PrgmData

_PrgmData:
	ld	iy, 0D00080h
	ld	hl, (0D0118Ch)
	push	hl
	ld	hl, 0
	call	0020798h
	set	1, (iy+8)
	call	0020F00h
	call	002079Ch
	res	1, (iy+8)
	pop	hl
	ld	(0D0118Ch), hl
	ret
