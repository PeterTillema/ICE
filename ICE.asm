.nolist
#include "ti84pce.inc"
#include "defines.asm"
.list

.db tExtTok, tAsm84CeCmp
.org saveSScreen-18


start:
	ld de, saveSScreen
	ld hl, UserMem+ICEStart-start
	ld bc, ICEStop-ICEStart
	ldir
	jp saveSScreen
ICEStart:
	call _RunIndicOff
	call _AnsName
	call _FindSym
	ret c
	ex de, hl
	ld a, (hl)
	cp 9
	ret nc													; return if more than 8 characters
	ld bc, 0
	ld c, a
	inc hl
	inc hl
	ld de, OP1+1
	push bc
		ldir
		ld hl, OP1
		ld (hl), ProgObj									; OP1 = program name
		call _ChkFindSym
	pop bc
	ret c
	scf
	sbc hl, hl
	ld (hl), 2
	call _ChkInRam
	jr nc, InRam
InArchive:
	ld hl, 10
	add hl, de
	add hl, bc
	ex de, hl
InRam:
	ex de, hl
	ld c, (hl)												; BC = program length
	inc hl
	ld b, (hl)
	inc hl
	ld (curPC), hl
	push hl
	pop de
	add hl, bc
	dec hl
	ld (endPC), hl
	ld hl, output
	ld (hl), 0
	push hl
	pop de
	inc de
	ld bc, 7000
	ldir													; clear some memory
	ld hl, stack											; set all the stacks + pointers... :O
	ld (stackPtr), hl
	ld hl, output
	ld (outputPtr), hl
	ld hl, programData
	ld (programPtr), hl
	ld hl, labelStack
	ld (labelPtr), hl
	ld hl, gotoStack
	ld (gotoPtr), hl
	ld hl, programDataData
	ld (programDataDataPtr), hl
	ld hl, programDataOffsetStack
	ld (programDataOffsetPtr), hl
	ld a, 8
	ld (programNameLength), a
	ld hl, 00021DDh
	call InsertHL											; ld ix, XX****
	call InsertA											; ld ix, XXXX**
	ld a, 0E3h
	call InsertA											; ld ix, XXXXXX
	call _CurFetch
	cp tii
	jr nz, standard_name
	ld hl, varname2
	ld (varname_offset), hl
name_loop:
	push hl
		call _IncFetch
	pop hl
	jr c, StartParse
	inc hl
	cp 041h													; only A-Z allowed - will be changed soon
	jr c, StartParse
	cp 05Ch
	jr nc, StartParse
	ld (hl), a
	ld a, (programNameLength)
	dec a
	ld (programNameLength), a								; (b is already used, can't use "djnz")
	jr nz, name_loop
	jr StartParse
standard_name:
	ld hl, varname
	ld (varname_offset), hl
StartParse:
	res has_already_input, (iy+myFlags)						; only one times the Input routine in the program data
ParseProgram:
ParseToEnd:
	call ParseLine
	call _CurFetch
	jp c, FindGotos
	cp tEnd
	jr nz, ParseProgram
	ld a, (AmountOfEnds)
	or a
	jp z, EndError
	ld hl, AmountOfEnds
	dec (hl)
	jp _IncFetch
ParseLine:
	call _CurFetch
	cp tEnd
	ret z
	cp tEnter
	jp z, _IncFetch
	ld hl, functions
	ld bc, functions_stop-functions
	cpir													; check which function (Repeat, While, Disp, ClrHome...) it is...
	jp nz, ParseExpression
	ld b, 3
	mlt bc
	ld hl, functions_start
	add hl, bc
	ld hl, (hl)
	jp (hl)													; jump to that function
FindGotos:
	ld hl, (gotoPtr)
	ld de, gotoStack
	or a
	sbc hl, de
	jr z, AddDataToProgramData								; have we finished all the Goto's?
	add hl, de
	dec hl
	dec hl
	dec hl
	push hl
		ld hl, (hl)
		ex de, hl											; de = pointer to goto data
		ld hl, (labelPtr)
FindLabels:
		ld bc, labelStack
		or a
		sbc hl, bc
		jr z, StopFindLabels								; have we finished all the Lbl's?
		add hl, bc
		dec hl
		dec hl
		dec hl												; hl = pointer to label
FindLabel:
		push hl
			push de
				ld hl, (hl)
				call CompareStrings							; is it the right label?
			pop de
		pop hl
		jr nz, LabelNotRightOne
RightLabel:
		dec hl
		dec hl
		dec hl
		ld hl, (hl)
		ex de, hl											; de = pointer to label memory
	pop hl													; hl = pointer to goto
	dec hl
	dec hl
	dec hl
	ld hl, (hl)												; hl = pointer to jump to
	ld (hl), de
	ld hl, (gotoPtr)
	ld de, -6
	add hl, de												; get next Goto
	ld (gotoPtr), hl
	jr FindGotos
LabelNotRightOne:
		dec hl
		dec hl
		dec hl
		jr FindLabels
StopFindLabels:
	pop hl
AddDataToProgramData:
	ld a, 0C9h
	call InsertA											; ret
	ld hl, (programDataDataPtr)
	ld bc, programDataData
	or a
	sbc hl, bc
	push hl
	pop bc													; bc = data length
	jr z, CreateProgram										; check IF there is data
	ld de, (programPtr)
	ld hl, programDataData
	or a
	sbc hl, de
	push hl
		add hl, de
		ldir												; move the data to the end of the program
		ld (programPtr), de
	pop de
	ld hl, (programDataOffsetPtr)
AddDataLoop:												; update all the pointers pointing to data
	ld bc, programDataOffsetStack
	or a
	sbc hl, bc
	jr z, CreateProgram										; no more pointers left
	add hl, bc
	dec hl
	dec hl
	dec hl
	push hl
		ld hl, (hl)											; complicated stuff XD
		ld (SMC3), hl
		ld hl, (hl)
		or a
		sbc hl, de
	pop bc
SMC3 = $+1
	.db 022h												; ld (******), hl
	.dl 0													; ld (XXXXXX), hl
	push bc
	pop hl
	jr AddDataLoop
CreateProgram:
	ld hl, (varname_offset)
	call _Mov9ToOP1
	call _ChkFindSym
	jr c, CreateFile
NeedToDeleteFile:
	call _DelVar
CreateFile:
	ld hl, (programPtr)
	ld bc, programData
	or a
	sbc hl, bc
	push hl
		inc hl
		inc hl												; add header
		call _CreateProtProg
	pop bc
	inc de
	inc de
	ld hl, programData
	ex de, hl
	ld (hl), 0EFh											; insert header
	inc hl
	ld (hl), 07Bh
	inc hl
	ex de, hl
	ldir													; insert the program data
	call _DrawStatusBar
	call _HomeUp
	call _ClrLCDFull										; ready :) :) :)
	ld hl, GoodCompileMessage
	jp _PutS
	
#include "functions.asm"
#include "parse.asm"
#include "data.asm"

#include "operators/add.asm"
#include "operators/sub.asm"
#include "operators/mul.asm"
#include "operators/div.asm"
#include "operators/ge.asm"
#include "operators/le.asm"
#include "operators/gt.asm"
#include "operators/lt.asm"
#include "operators/eq.asm"
#include "operators/ne2.asm"
#include "operators/or.asm"
#include "operators/xor.asm"
#include "operators/and.asm"
#include "operators/sto.asm"

#include "functions/pause.asm"
#include "functions/clrhome.asm"
#include "functions/disp.asm"
#include "functions/repeat.asm"
#include "functions/while.asm"
#include "functions/if.asm"
#include "functions/asm.asm"
#include "functions/inc.asm"
#include "functions/dec.asm"
#include "functions/label.asm"
#include "functions/goto.asm"
#include "functions/input.asm"

ICEStop:

.echo $-start
