assume adl = 1
public _SetHooks

_SetHooks:
	pop	bc
	pop	hl			; HL = data pointer
	ld	iy, 0D00080h
	call	00213CCh		; _SetGetKeyHook
	ld	de, 697
	add	hl, de
	call	00213F8h		; _SetTokenHook
	ld	de, 32
	add	hl, de
	call	00213C4h		; _SetCursorHook
	push	hl
	push	bc
	ret
