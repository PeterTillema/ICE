assume adl = 1
public _SqrtData

_SqrtData:
	dec	sp			; (sp) = ?
	push	hl			; (sp) = ?uhl
	dec	sp			; (sp) = ?uhl?
	pop	iy			; (sp) = ?u, uix = hl?
	dec	sp			; (sp) = ?u?
	pop	af			; af = u?
	or	a, a
	sbc	hl, hl
	ex	de, hl			; de = 0
	sbc	hl, hl			; hl = 0
	ld	bc, 0C40h		; b = 12, c = 0x40
Sqrt24Loop:
	sub	a, c
	sbc	hl, de
	jr	nc, Sqrt24Skip
	add	a, c
	adc	hl, de
Sqrt24Skip:
	ccf
	rl	e
	rl	d
	add	iy, iy
	rla
	adc	hl, hl
	add	iy, iy
	rla
	adc	hl, hl
	djnz	Sqrt24Loop
	ret
