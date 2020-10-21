assume adl = 1
public _GotoEditor

include 'ti84pce.inc'

extern __exitsp

_GotoEditor:
	ld	iy, flags
	call	_DrawStatusBar
	pop	bc
	pop	hl
	pop	de
	ld.sis	(errOffset and 0FFFFh), de
	ld	de, progToEdit
	ld	bc, 8
	push	hl
	ldir
	pop	hl
	
; This is necessary because Cesium always assumes that OP1 is pushed before going into the editor
	call	_Mov9ToOP1
	call	_PushOP1
	
; Cleanup C things
	ld	sp, (__exitsp + 1);
	pop	iy
	pop	af
	ex	(sp), hl
	ld	(hl), a
	call	_usb_ResetTimer
	pop	hl
	
	ld	de, plotSScreen
	ld	hl, StartProgramEditor
	ld	bc, StopProgramEditor - StartProgramEditor
	ldir
	jp	plotSScreen
StartProgramEditor:
	ld	hl, userMem
	ld	de, (asm_prgm_size)
	call	_DelMem
	ld	a, kPrgmEd
	call	_NewContext
	ld.sis	bc, (errOffset and 0FFFFh)
	ld	hl, (editTail)
	ld	de, (editCursor)
	ld	a, b
	or	a, c
	jr	z, FindPreviousEnter
	ldir
	ld	(editTail), hl
	ld	(editCursor), de
FindPreviousEnter:
	call	_BufLeft
	jr	z, AtTopOfProgram
	ld	a, e
	cp	a, tEnter
	jr	z, AtStartOfLine
	inc	bc
	jr	FindPreviousEnter
AtStartOfLine:
	call	_BufRight
AtTopOfProgram:
	push	bc
	call	_ClrWindow
	ld	hl, 0000001h
	ld	(curRow), hl
	ld	a, ':'
	call	_PutC
	call	_DispEOW
	pop	bc
FindCursor:
	ld	a, b
	or	a, c
	jp	z, _Mon
	call	_CursorRight
	dec	bc
	jr	FindCursor
StopProgramEditor: