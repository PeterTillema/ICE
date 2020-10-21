assume adl = 1
public _TimerData

_TimerData:
	ld	hl, 0F20031h
	set	1, (hl)			; Set count up
	dec	hl
	set	1, (hl)			; Set 32KHz crystal timer
	set	0, (hl)			; Enable
	ld	l, h
	ld	hl, (hl)		; Return current time
	ret
