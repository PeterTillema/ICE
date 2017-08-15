OpenEditBuffer:
	ld	hl, (curPC)
	ld	de, (begPC)
	or	a, a
	sbc	hl, de
	ld.sis	(errOffset - 0D00000h), hl
	ld	hl, OP1+1
	ld	de, progToEdit
	call	_Mov8b
	ld	a, 046h
	jp	_NewContext