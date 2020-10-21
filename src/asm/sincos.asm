assume adl = 1
public _SincosData

_SincosData:
_FP_Cos:
	ld	a, l
	add	a, 64
	ld	l, a
_FP_Sin:
	ld	a, l
	ld	b, a
	and	a, 000111111b
	bit	6, b
	jr	z, _FP_Sin_Change
	add	a, -65
	cpl
_FP_Sin_Change:
	ld	de, 0
	sbc	hl, hl
	ld	l, a
	ex	de, hl
	add	hl, de
	bit	7, b
	ld	e, (hl)
	ret	z
	sbc	hl, hl
	sbc	hl, de
	ex	de, hl
	ret

SinTable:
	db	000h, 006h, 00Ch, 012h, 018h, 01Fh, 025h, 02Bh, 031h, 037h, 03Dh, 044h, 04Ah, 04Fh, 055h, 05Bh
	db	061h, 067h, 06Dh, 072h, 078h, 07Dh, 083h, 088h, 08Dh, 092h, 097h, 09Ch, 0A1h, 0A6h, 0ABh, 0AFh
	db	0B4h, 0B8h, 0BCh, 0C1h, 0C5h, 0C9h, 0CCh, 0D0h, 0D4h, 0D7h, 0DAh, 0DDh, 0E0h, 0E3h, 0E6h, 0E9h
	db	0EBh, 0EDh, 0F0h, 0F2h, 0F4h, 0F5h, 0F7h, 0F8h, 0FAh, 0FBh, 0FCh, 0FDh, 0FDh, 0FEh, 0FEh, 0FEh
	db	0FFh
