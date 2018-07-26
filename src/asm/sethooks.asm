.assume adl = 1
segment data
.def _SetHooks1
.def _SetHooks2

_SetHooks1:
	pop	bc
	pop	hl			; HL = data pointer
	ld	iy, 0D00080h
	call	00213CCh		; _SetGetKeyHook
	ld	de, 696
	add	hl, de
	call	00213F8h		; _SetTokenHook
	ld	de, 32
	add	hl, de
	call	00213C4h		; _SetCursorHook
	push	hl
	push	bc
	ret
	
_SetHooks2:
	pop	bc
	pop	hl			; HL = data pointer
	ld	iy, 0D00080h
	call	0021418h		; _SetWindowHook
	push	hl
	push	bc
	ret
