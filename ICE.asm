#include "ti84pce.inc"
#include "defines.asm"
;#include "app.inc"

.db tExtTok, tAsm84CeCmp
.org UserMem

start:
    ;app_create()
    ;ret
    
;app_start("ICE", "(c) 2017")
	push	ix
	ld	(backupSP), sp
	ld	hl, (begPC)
	ld	(backupBegPC), hl
	ld	hl, (curPC)
	ld	(backupCurPC), hl
	ld	hl, (endPC)
	ld	(backupEndPC), hl
	call	_RunIndicOff
	call	InstallHooks
GUI:
	ld	a, lcdBpp8
	ld	(mpLcdCtrl), a
	ld	hl, mpLcdPalette
	ld	b, 0
_:	ld	d, b
	ld	a, b
	and	011000000b
	srl	d
	rra
	ld	e, a
	ld	a, 000011111b
	and	a, b
	or	a, e
	ld	(hl), a
	inc	hl
	ld	(hl), d
	inc	hl
	inc	b
	jr	nz, -_
	ld	hl, vRAM
	ld	(hl), 189
	push	hl
	pop	de
	inc	de
	ld	bc, 320*10
	ldir
	ld	(hl), 0
	ld	bc, 320
	ldir
	ld	(hl), 255
	ld	bc, 320*229-1
	ldir
	set	good_compilation, (iy+fProgram1)
	res	modified_iy, (iy+fAlways1)
	res	debug_on, (iy+fAlways1)
	ld	hl, ICEName
	ld	a, 1
	ld	(TextYPos_ASM), a
	add	a, 20
	ld	(TextXPos_ASM), a
	call	PrintString
	ld	hl, TextYPos_ASM
	inc	(hl)
	inc	(hl)
	ld	hl, (progPtr)
FindPrograms:
	call	FindNextGoodVar
	jr	nz, StopFindingPrograms
	push	hl
	ld	a, (TextYPos_ASM)
	add	a, 10
	jr	c, +_
	ld	(TextYPos_ASM), a
	ld	hl, 10
	ld	(TextXPos_ASM), hl
	ld	hl, AmountOfPrograms
	inc	(hl)
	call	_ChkInRAM
	ld	a, '#'
	call	c, _PrintChar_ASM
	ld	hl, (ProgramNamesPtr)
	ld	de, -8
	add	hl, de
	call	PrintString
_:	pop	hl
	jr	FindPrograms
StopFindingPrograms:
	ld	a, 13
	ld	(TextYPos_ASM), a
	ld	hl, 1
	ld	(TextXPos_ASM), hl
	ld	a, (AmountOfPrograms)
	or	a, a
	jp	z, NoProgramsError
	ld	(AmountPrograms), a
	ld	l, 1
PrintCursor:
	ld	e, l
	ld	d, 10
	mlt	de
	inc	e
	inc	e
	inc	e
	ld	a, e
	ld	(TextYPos_ASM), a
	xor	a, a
	ld	(color), a
	inc	a
	ld	(TextXPos_ASM), a
	ld	a, '>'
	call	_PrintChar_ASM
	ld	a, 255
	ld	(color), a
	ld	a, 1
	ld	(TextXPos_ASM), a
_:	push	hl
	call	_GetCSC
	pop	hl
	or	a, a
	jr	z, -_
	cp	a, skUp
	jr	z, PressedUp
	cp	a, skDown
	jr	z, PressedDown
	cp	a, skClear
	jp	z, StopProgram
	cp	a, skEnter
	jr	z, PressedEnter
	cp	a, skAdd
	jr	nz, -_
PressedPlus:
	set	debug_on, (iy+fAlways1)
PressedEnter:
	dec	l
	ld	h, 8
	mlt	hl
	ld	de, ProgramNamesStack-1
	add	hl, de
	call	_Mov9ToOP1
	jr	StartParsing
PressedUp:
	ld	a, l
	dec	a
	jr	z, -_
	dec	l
	ld	a, 23
	call	_PrintChar_ASM
	jr	PrintCursor
PressedDown:
	ld	a, l
AmountPrograms = $+1
	cp	a, 0
	jr	z, -_
	inc	l
	ld	a, 23
	call	_PrintChar_ASM
	jr	PrintCursor
StartParsing:
	ld	a, ProgObj
	ld	(OP1), a
_:	call	_ChkFindSym
	jr	nc, +_
	ld	hl, OP1
	inc	(hl)
	jr	-_
_:	call	_ChkInRAM
	jr	nc, +_
	ex	de, hl
	ld	de, 9
	add	hl, de
	ld	e, (hl)
	add	hl, de
	inc	hl
	ex	de, hl
_:	ld	bc, 0
	ex	de, hl
	ld	c, (hl)			; BC = program length
	inc	hl
	ld	b, (hl)
	inc	hl
	ld	(curPC), hl
	ld	(begPC), hl
	add	hl, bc
	dec	hl
	ld	(endPC), hl
	call	PrintCompilingProgram
	ld	(iy+fProgram1), 1
	ld	(iy+fProgram2), 0
	ld	hl, CData
	ld	de, (programPtr)
	ld	bc, CData2 - CData
	ldir
	ld	(programPtr), de
	call	PreScanPrograms
	ld	a, 0CDh
	ld	hl, _RunIndicOff
	call	InsertAHL		; call _RunIndicOff
	ld	hl, (programPtr)
	ld	de, 4+4+5+UserMem-program
	add	hl, de
	call	InsertAHL		; call *
	ld	bc, 08021FDh
	ld	de, 0C3D000h
	ld	hl, _DrawStatusBar
	call	InsertBCDEHL		; ld iy, flags \ jp _DrawStatusBar
	ld	hl, (programPtr)
	ld	(PrevProgramPtr), hl
	ld	a, (amountOfCRoutines)
	or	a, a
	jr	nz, CompileProgramFull
	res	comp_with_libs, (iy+fProgram1)
	ld	hl, program+5
	ld	(programPtr), hl
CompileProgramFull:
	ld	a, (AmountOfSubPrograms)
	or	a, a
	jr	nz, SkipGetProgramName
	ld	hl, varname
	call	GetProgramName
	ld	hl, OP1+1
	ld	de, varname+1
	ld	b, 8
CheckNames:
	ld	a, (de)
	or	a, a
	jr	z, CheckNamesSameLength
	cp	a, (hl)
	jr	nz, GoodProgramName
	inc	hl
	inc	de
	djnz	CheckNames
CheckNamesSameLength:
	cp	a, (hl)
	jp	z, SameNameError
GoodProgramName:
SkipGetProgramName:
ParseProgramUntilEnd:
CompileLoop:
	xor	a, a
	ld	(iy+fExpression1), a
	ld	(iy+fExpression2), a
	ld	(iy+fExpression3), a
	ld	(iy+fFunction1), a
	ld	(iy+fFunction2), a
	ld	(openedParensE), a
	ld	(openedParensF), a
	call	_IncFetch
	ld	(tempToken), a
	jr	c, FindGotos
	cp	a, tEnd
	jr	nz, ++_
_:	ld	hl, amountOfEnds
	ld	a, (hl)
	dec	(hl)
	or	a, a
	jp	z, EndError
	ld	a, (tempToken)
	ret
_:	cp	a, tElse
	jr	z, --_
	call	ParseLine
	ld	hl, (curPC)
	ld	de, (begPC)
	or	a, a
	sbc	hl, de
	ld	bc, 320
	call	__imulu
	push	hl
	ld	hl, (endPC)
	or	a, a
	sbc	hl, de
	inc	hl
	push	hl
	pop	bc
	pop	hl
	call	__idivu
	push	hl
	pop	bc
	ld	a, b
	or	a, c
	jr	z, +_
	ld	hl, vRAM+(320*220)
	ld	(hl), 0
	push	hl
	pop	de
	inc	de
	dec	bc
	ld	a, b
	or	a, c
	jr	z, CompileLoop
	ldir
_:	jr	CompileLoop
FindGotos:
	ld	hl, amountOfEnds
	ld	a, (hl)
	or	a, a
	jr	z, +_
	dec	(hl)
	ret
_:	ld	hl, AmountOfSubPrograms
	ld	a, (hl)
	dec	(hl)
	or	a, a
	ret	nz
FindGotosLoop:
	ld	hl, (gotoPtr)
	ld	de, gotoStack
	or	a, a
	sbc	hl, de
	jr	z, AddDataToProgramData	; have we finished all the Goto's?
	add	hl, de
	dec	hl
	dec	hl
	dec	hl
	push	hl
	ld	hl, (hl)
	ex	de, hl			; de = pointer to goto data
	ld	hl, (labelPtr)
FindLabels:
	ld	bc, labelStack
	or	a, a
	sbc	hl, bc
	jp	z, LabelError		; have we finished all the Lbl's?
	add	hl, bc
	dec	hl
	dec	hl
	dec	hl			; hl = pointer to label
FindLabel:
	push	hl
	push	de
	ld	hl, (hl)		; hl = pointer to label data
	call	CompareStrings		; is it the right label?
	pop	de
	pop	hl
	jr	nz, LabelNotRightOne
RightLabel:
	dec	hl
	dec	hl
	dec	hl
	ld	hl, (hl)
	ld	de, UserMem-program
	add	hl, de
	ex	de, hl			; de points to label memory
	pop	hl			; hl = pointer to goto
	dec	hl
	dec	hl
	dec	hl
	ld	hl, (hl)		; hl = pointer to jump to
	ld	(hl), de
	ld	hl, (gotoPtr)
	ld	de, -6
	add	hl, de			; get next Goto
	ld	(gotoPtr), hl
	jr	FindGotosLoop
LabelNotRightOne:
	dec	hl
	dec	hl
	dec	hl
	jr	FindLabels
StopFindLabels:
	pop	hl
AddDataToProgramData:
	bit	last_token_is_ret, (iy+fExpression1)
	jr	nz, +_
	ld	a, 0C9h
	call	InsertA			; ret
_:	ld	hl, (programDataDataPtr)
	ld	bc, programDataData
	or	a, a
	sbc	hl, bc
	push	hl
	pop	bc			; bc = data length
	jr	z, CreateProgram	; check IF there is data
	ld	de, (programPtr)
	ld	hl, programDataData
	or	a, a
	sbc	hl, de
	push	hl
	add	hl, de
	ldir				; move the data to the end of the program
	ld	(programPtr), de
	pop	de
	ld	hl, (programDataOffsetPtr)
AddDataLoop:				; update all the pointers pointing to data
	ld	bc, programDataOffsetStack
	or	a, a
	sbc	hl, bc
	jr	z, CreateProgram	; no more pointers left
	add	hl, bc
	dec	hl
	dec	hl
	dec	hl
	push	hl
	ld	hl, (hl)		; complicated stuff XD
	push	hl
	ld	hl, (hl)
	or	a, a
	sbc	hl, de
	ld	bc, UserMem-program
	add	hl, bc
	push	hl
	pop	bc
	pop	hl
	ld	(hl), bc		; ld (XXXXXX), hl
	pop	hl
	jr	AddDataLoop
CreateProgram:
	ld	hl, varname
	call	_Mov9ToOP1
	call	_ChkFindSym
	jr	c, ++_
	call	_ChkInRAM
	jr	nc, +_
	call	_Arc_Unarc
	ld	bc, 5
	add	hl, bc
_:	call	_DelVar
_:	ld	hl, (programPtr)
	ld	bc, program
	or	a, a
	sbc	hl, bc
	push	hl
	ld	bc, 17
	add	hl, bc
	push	hl
	call	_EnoughMem
	ld	hl, NotEnoughMem
	jp	c, DispFinalString
	pop	hl
	ld	bc, -15
	add	hl, bc
	call	_CreateProtProg
	pop	bc
	inc	de
	inc	de
	ld	hl, program
	ex	de, hl
	ld	(hl), tExtTok		; insert header
	inc	hl
	ld	(hl), tAsm84CeCmp
	inc	hl
    ;ld      (hl), 07Fh			; Cesium recognition
    ;inc     hl
	ex	de, hl
	ldir				; insert the program data
	ld	hl, GoodCompileMessage
	set	good_compilation, (iy+fProgram1)
	jp	DispFinalString		; DONE :D :D :D

#include "routines.asm"
#include "parse.asm"
#include "putchar.asm"
#include "programs.asm"
#include "hooks.asm"
#include "operators functions/functions.asm"
#include "operators functions/operators.asm"
#include "operators functions/function_for.asm"
#include "operators functions/function_C.asm"
#include "clibs/graphics.asm"
varname:
	.db	ProtProgObj, 0,0,0,0,0,0,0,0,0
    
usedCroutines:
.fill       AMOUNT_OF_C_FUNCTIONS, 0
;app_data()
#include "data.asm"
;app_end()