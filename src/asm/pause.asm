assume adl = 1
public _PauseData

_PauseData:
	di
	dec	hl
_PauseLoop:
	ld	c, 110
_PauseInnerLoop1:
	ld	b, 32
_PauseInnerLoop2:
	djnz	_PauseInnerLoop2
	dec	c
	jr	nz, _PauseInnerLoop1
	or	a, a
	ld	de, -1
	add	hl, de
	jr	c, _PauseLoop
	ret
