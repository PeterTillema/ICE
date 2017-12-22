.assume adl = 1
segment data
.def _RunPrgm

_RunPrgm:
	ld	a, 2
	ld	(-1), a
	ld	iy, 0D00080h		; flags
	;call	0021A3Ch		; _DrawStatusBar
	call	00004F0h		; _usb_DisableTimer
	
	ld	de, 0D0EA1Fh		; plotSScreen
	ld	hl, StartRunProgram
	ld	bc, EndRunProgram - StartRunProgram
	ldir
	
	inc	sp
	inc	sp
	inc	sp
	pop	hl
	ld	de, 0D0065Bh		; progToEdit
	ld	c, 8
	ldir
	
	jp	0D0EA1Fh		; plotSScreen
	
StartRunProgram:
	ld	hl, 0D1A881h		; userMem
	ld	de, (0D0118Ch)		; asm_prgm_size
	call	0020590h		; _DelMem
	ld	sp, (0D007FAh)		; onSP
	ld	a, 040h			; cxCmd
	jp	002016Ch		; _NewContext
EndRunProgram:
