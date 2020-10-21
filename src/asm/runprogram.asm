assume adl = 1
public _RunPrgm

extern __exitsp

_RunPrgm:
	call	0021A3Ch		; _DrawStatusBar
	ld	de, 0D09466h		; plotSScreen
	ld	hl, StartRunProgram
	ld	bc, EndRunProgram - StartRunProgram
	ldir
	jp	0D09466h		; plotSScreen
	
StartRunProgram:
; Copy the name to OP1
	pop	hl
	pop	hl			; HL points to the program name
	ld	de, 0D005F9h		; OP1 + 1
	push	de
	ld	bc, 8
	ldir
	pop	hl
	dec	hl
	ld	(hl), 5			; ProgObj
	
; Cleanup C things
	ld	sp, (__exitsp + 1);
	pop	iy
	pop	af
	ex	(sp), hl
	ld	(hl), a
	call	00004F0h		; usb_ResetTimers
	pop	hl
	
; Remove ICE from UserMem
	ld	hl, 0D1A881h		; userMem
	ld	de, (0D0118Ch)		; asm_prgm_size
	call	0020590h		; _DelMem
	
; Copy the new program to UserMem and jump to it
	call	002050Ch		; _ChkFindSym
	ex	de, hl
	call	0021D9Ch		; _LoadDEInd_s
	inc	hl
	inc	hl
	push	hl
	push	de
	ex	de, hl
	call	002072Ch		; _ErrNotEnoughMem
	pop	hl
	ld	(0D0118Ch), hl		; asm_prgm_size
	ld	de, 0D1A881h		; userMem
	push	de
	call	0020514h		; _InsertMem
	pop	de
	pop	hl
	ld	bc, (0D0118Ch)		; asm_prgm_size
	add	hl, bc
	ldir
	
; An finally run the program!
	jp	0D1A881h		; userMem
EndRunProgram: