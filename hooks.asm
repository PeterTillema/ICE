InstallHooks:
	ld	hl, ICEAppvar
	call	_Mov9ToOP1
	call	_ChkFindSym
	jr	c, ++_
	call	_ChkInRAM
	jr	nc, +_
	call	_Arc_Unarc
	ld	bc, 5
	add	hl, bc
_:	call	_DelVar
_:	ld	hl, Hooks_end - KeyHook_start
	call	_CreateAppVar
	inc	de
	inc	de
	ld	hl, KeyHook_start
	ld	bc, Hooks_end - KeyHook_start
	ldir
	call	_OP4ToOP1
	call	_Arc_Unarc
	call	_ChkFindSym
	ld	hl, 19			; archived program header+VAT entry
	add	hl, de
	call	_SetGetKeyHook
    
	ld	de, KeyHook_end - KeyHook_start
	add	hl, de
	call	_SetTokenHook
    
	ld	de, TokenHook_end - TokenHook_start
	add	hl, de
	jp	_SetCursorhook
    
KeyHook_start:
	.db	83h
	or	a, a
	ret	z
	ld	b, a
	ld	a, (cxCurApp)
	cp	a, cxPrgmEdit
	ld	a, b
	ret	nz
	push	af
	call	_os_ClearStatusBarLow
	res	displayed_det, (iy+fAlways1)
	pop	af
	cp	a, kTrace
	ret	nz
DisplayCustomTokensAndCFunctions:
	call	_CursorOff
	ld	d, 0
DisplayTabWithTokens:
	push	de
	call	_ClrLCDFull
	pop	de
	ld	hl, 30
	ld	(penRow), hl
	ld	hl, 12
	ld	(penCol), hl
	ld	b, 0
	ld	a, d
	ld	e, 3
	mlt	de
	ld	hl, TabData - KeyHook_start
	add	hl, de
	ld	de, (rawKeyHookPtr)
	add	hl, de
	ld	hl, (hl)
	add	hl, de
	ld	d, a
	ld	e, 0
	jr	DisplayTokensLoop
KeyIsLeft:
	ld	a, d
	or	a, a
	jr	z, KeyLoop
	dec	d
	jr	DisplayTabWithTokens
KeyIsRight:
	ld	a, d
	cp	a, 5
	jr	z, KeyLoop
	inc	d
	jr	DisplayTabWithTokens
DisplayTokensLoop:
	ld	a, b
	cp	a, 16
	jr	z, StopDisplayingTokens
	inc	b
	call	_VPutS
	push	hl
	push	de
	ld	hl, (penRow)
	ld	de, 13
	add	hl, de
	ld	(penRow), hl
	ld	hl, 12
	ld	(penCol), hl
	pop	de
	pop	hl
	ld	a, (hl)
	or	a, a
	jr	nz, DisplayTokensLoop
StopDisplayingTokens:
	ld	hl, 1
	ld	(penCol), hl
GetRightCustomToken:
	ld	a, e
	ld	b, d
	ld	d, 13
	mlt	de
	ld	hl, 30
	add	hl, de
	ld	d, b
	ld	e, a
	ld	(penRow), hl
	ld	hl, 1
	ld	(penCol), hl
	push	hl
	push	de
	ld	a, '>'
	call	_VPutMap
	pop	de
	pop	hl
	ld	(penCol), hl
KeyLoop:
	call	_GetCSC
	or	a, a
	jr	z, KeyLoop
	cp	a, skLeft
	jr	z, KeyIsLeft
	cp	a, skRight
	jr	z, KeyIsRight
	cp	a, skUp
	jr	nz, KeyNotUp
	ld	a, e
	or	a, a
	jr	z, KeyLoop
	dec	e
EraseCursor:
	push	de
	ld	a, ' '
	call	_VPutMap
	ld	a, ' '
	call	_VPutMap
	ld	a, ' '
	call	_VPutMap
	pop	de
	jr	GetRightCustomToken
KeyNotUp:
	cp	a, skDown
	jr	nz, KeyNotDown
	ld	a, d
	cp	a, 5
	ld	a, e
	jr	nz, +_
	cp	a, (AMOUNT_OF_C_FUNCTIONS + AMOUNT_OF_CUSTOM_TOKENS)%16 - 1
	jr	z, KeyLoop
_:	ld	a, e
	cp	a, 16-1
	jr	z, KeyLoop
	inc	e
	jr	EraseCursor
KeyNotDown:
	cp	a, skClear
	jr	z, KeyIsClear
	cp	a, skEnter
	jr	nz, KeyLoop
	ld	a, e
	ld	e, 16
	mlt	de
	add	a, e
	sub	a, AMOUNT_OF_CUSTOM_TOKENS
	jr	c, InsertCustomToken
	ld	hl, saveSScreen
	ld	(hl), tDet
	inc	hl
	cp	a, 10
	jr	c, +_
	ld	d, a
	ld	e, 10
	xor	a, a
	ld	b, 8
_loop:
	sla	d
	rla
	cp	a, e
	jr	c, $+4
	sub	a, e
	inc	d
	djnz	_loop
	ld	e, a
	ld	a, d
	add	a, t0
	ld	(hl), a
	inc	hl
	ld	a, e
_:	add	a, t0
	ld	(hl), a
	inc	hl
	ld	(hl), 0
	ld	hl, saveSScreen
InsertCFunctionLoop:
	ld	a, (hl)
	or	a, a
	jr	z, BufferSearch
	ld	de, (editTail)
	ld	a, (de)
	cp	a, tEnter
	ld	d, 0
	ld	e, (hl)
	jr	z, +_
	push	hl
	call	_BufReplace
	pop	hl
	inc	hl
	jr	InsertCFunctionLoop
_:	push	hl
	call	_BufInsert
	pop	hl
	inc	hl
	jr	InsertCFunctionLoop
InsertCustomToken:
	add	a, 10+AMOUNT_OF_CUSTOM_TOKENS
	ld	e, a
	ld	d, tVarOut
	ld	hl, (editCursor)
	ld	a, (hl)
	cp	a, tEnter
	jr	z, +_
	call	_BufReplace
	jr	BufferSearch
_:	call	_BufInsert
KeyIsClear:
BufferSearch:
	ld	bc, 0
_:	call	_BufLeft
	jr	z, BufferFound
	ld	a, e
	cp	a, tEnter
	jr	z, +_
	inc	bc
	jr	-_
_:	call	_BufRight
BufferFound:
	push	bc
	call	_ClrLCDFull
	call	_ClrTxtShd
	ld	de, CustomTokensProgramText - KeyHook_start
	ld	hl, (rawKeyHookPtr)
	add	hl, de
	xor	a, a
	ld	(curCol), a
	ld	(curRow), a
	call	_PutS
	ld	hl, progToEdit
	ld	b, 8
_:	ld	a, (hl)
	or	a, a
	jr	z, +_
	call	_PutC
	inc	hl
	djnz	-_
_:	call	_NewLine
	ld	a, ':'
	call	_PutC
	call	_DispEOW
	pop	bc
MoveCursorOnce:
	ld	a, b
	or	a, c
	jr	z, ReturnToEditor
	call	_CursorRight
	dec	bc
	jr	MoveCursorOnce
ReturnToEditor:
	call	_CursorOn
	inc	a			; reset zero flag
	ld	a, 0
	ret
CustomTokensData:
Tab1:
C1: .db "ExecHex(", 0
C2: .db "DefineSprite(", 0
C3: .db "Call ", 0
C4: .db "CompilePrgm(", 0
C5: .db "SetBASICVar(", 0
C6: .db "GetBASICVar(", 0

#define AMOUNT_OF_C_FUNCTIONS 84

C6_:  .db "Begin", 0
C7_:  .db "End", 0
C8_:  .db "SetColor", 0
C9_:  .db "SetDefaultPalette", 0
C10_: .db "SetPalette", 0
C11_: .db "FillScreen", 0
C12_: .db "SetPixel", 0
C13_: .db "GetPixel", 0
C14_: .db "GetDraw", 0
C15_: .db "SetDraw", 0
Tab2:
C16_: .db "SwapDraw", 0
C17_: .db "Blit", 0
C18_: .db "BlitLines", 0
C19_: .db "BlitArea", 0
C20_: .db "PrintChar", 0
C21_: .db "PrintInt", 0
C22_: .db "PrintUInt", 0
C23_: .db "PrintString", 0
C24_: .db "PrintStringXY", 0
C25_: .db "SetTextXY", 0
C26_: .db "SetTextBGColor", 0
C27_: .db "SetTextFGColor", 0
C28_: .db "SetTextTransparentColor", 0
C29_: .db "SetCustomFontData", 0
C30_: .db "SetCustomFontSpacing", 0
C31_: .db "SetMonospaceFont", 0
Tab3:
C32_: .db "GetStringWidth", 0
C33_: .db "GetCharWidth", 0
C34_: .db "GetTextX", 0
C35_: .db "GetTextY", 0
C36_: .db "Line", 0
C37_: .db "HorizLine", 0
C38_: .db "VertLine", 0
C39_: .db "Circle", 0
C40_: .db "FillCircle", 0
C41_: .db "Rectangle", 0
C42_: .db "FillRectangle", 0
C43_: .db "Line_NoClip", 0
C44_: .db "HorizLine_NoClip", 0
C45_: .db "VertLine_NoClip", 0
C46_: .db "FillCircle_NoClip", 0
C47_: .db "Rectangle_NoClip", 0
Tab4:
C48_: .db "FillRectangle_NoClip", 0
C49_: .db "SetClipRegion", 0
C50_: .db "GetClipRegion", 0
C51_: .db "ShiftDown", 0
C52_: .db "ShiftUp", 0
C53_: .db "ShiftLeft", 0
C54_: .db "ShiftRight", 0
C55_: .db "Tilemap", 0
C56_: .db "Tilemap_NoClip", 0
C57_: .db "TransparentTilemap", 0
C58_: .db "TransparentTilemap_NoClip", 0
C59_: .db "TilePtr", 0
C60_: .db "TilePtrMapped", 0
C61_: .db "LZDecompress", 0
C62_: .db "AllocSprite", 0
C63_: .db "Sprite", 0
Tab5:
C64_: .db "TransparentSprite", 0
C65_: .db "Sprite_NoClip", 0
C66_: .db "TransparentSprite_NoClip", 0
C67_: .db "GetSprite_NoClip", 0
C68_: .db "ScaledSprite_NoClip", 0
C69_: .db "ScaledTransparentSprite_NoClip", 0
C70_: .db "FlipSpriteY", 0
C71_: .db "FlipSpriteX", 0
C72_: .db "RotateSpriteC", 0
C73_: .db "RotateSpriteCC", 0
C74_: .db "RotateSpriteHalf", 0
C75_: .db "Polygon", 0
C76_: .db "Polygon_NoClip", 0
C77_: .db "FillTriangle", 0
C78_: .db "FillTriangle_NoClip", 0
C79_: .db "LZDecompressSprite", 0
Tab6:
C80_: .db "SetTextScale", 0
C81_: .db "SetTransparentColor", 0
C82_: .db "ZeroScreen", 0
C83_: .db "SetTextConfig", 0
C84_: .db "GetSpriteChar", 0
C85_: .db "Lighten", 0
C86_: .db "Darken", 0
C87_: .db "SetFontHeight", 0
C88_: .db "ScaledSprite", 0
C89_: .db "FloodFill", 0
	.db	0
TabData:
	.dl	Tab1 - KeyHook_start
	.dl	Tab2 - KeyHook_start
	.dl	Tab3 - KeyHook_start
	.dl	Tab4 - KeyHook_start
	.dl	Tab5 - KeyHook_start
	.dl	Tab6 - KeyHook_start
    
CData5:
	.dl	C6_ - KeyHook_start
	.dl	C7_ - KeyHook_start
	.dl	C8_ - KeyHook_start
	.dl	C9_ - KeyHook_start
	.dl	C10_ - KeyHook_start
	.dl	C11_ - KeyHook_start
	.dl	C12_ - KeyHook_start
	.dl	C13_ - KeyHook_start
	.dl	C14_ - KeyHook_start
	.dl	C15_ - KeyHook_start
	.dl	C16_ - KeyHook_start
	.dl	C17_ - KeyHook_start
	.dl	C18_ - KeyHook_start
	.dl	C19_ - KeyHook_start
	.dl	C20_ - KeyHook_start
	.dl	C21_ - KeyHook_start
	.dl	C22_ - KeyHook_start
	.dl	C23_ - KeyHook_start
	.dl	C24_ - KeyHook_start
	.dl	C25_ - KeyHook_start
	.dl	C26_ - KeyHook_start
	.dl	C27_ - KeyHook_start
	.dl	C28_ - KeyHook_start
	.dl	C29_ - KeyHook_start
	.dl	C30_ - KeyHook_start
	.dl	C31_ - KeyHook_start
	.dl	C32_ - KeyHook_start
	.dl	C33_ - KeyHook_start
	.dl	C34_ - KeyHook_start
	.dl	C35_ - KeyHook_start
	.dl	C36_ - KeyHook_start
	.dl	C37_ - KeyHook_start
	.dl	C38_ - KeyHook_start
	.dl	C39_ - KeyHook_start
	.dl	C40_ - KeyHook_start
	.dl	C41_ - KeyHook_start
	.dl	C42_ - KeyHook_start
	.dl	C43_ - KeyHook_start
	.dl	C44_ - KeyHook_start
	.dl	C45_ - KeyHook_start
	.dl	C46_ - KeyHook_start
	.dl	C47_ - KeyHook_start
	.dl	C48_ - KeyHook_start
	.dl	C49_ - KeyHook_start
	.dl	C50_ - KeyHook_start
	.dl	C51_ - KeyHook_start
	.dl	C52_ - KeyHook_start
	.dl	C53_ - KeyHook_start
	.dl	C54_ - KeyHook_start
	.dl	C55_ - KeyHook_start
	.dl	C56_ - KeyHook_start
	.dl	C57_ - KeyHook_start
	.dl	C58_ - KeyHook_start
	.dl	C59_ - KeyHook_start
	.dl	C60_ - KeyHook_start
	.dl	C61_ - KeyHook_start
	.dl	C62_ - KeyHook_start
	.dl	C63_ - KeyHook_start
	.dl	C64_ - KeyHook_start
	.dl	C65_ - KeyHook_start
	.dl	C66_ - KeyHook_start
	.dl	C67_ - KeyHook_start
	.dl	C68_ - KeyHook_start
	.dl	C69_ - KeyHook_start
	.dl	C70_ - KeyHook_start
	.dl	C71_ - KeyHook_start
	.dl	C72_ - KeyHook_start
	.dl	C73_ - KeyHook_start
	.dl	C74_ - KeyHook_start
	.dl	C75_ - KeyHook_start
	.dl	C76_ - KeyHook_start
	.dl	C77_ - KeyHook_start
	.dl	C78_ - KeyHook_start
	.dl	C79_ - KeyHook_start
	.dl	C80_ - KeyHook_start
	.dl	C81_ - KeyHook_start
	.dl	C82_ - KeyHook_start
	.dl	C83_ - KeyHook_start
	.dl	C84_ - KeyHook_start
	.dl	C85_ - KeyHook_start
	.dl	C86_ - KeyHook_start
	.dl	C87_ - KeyHook_start
	.dl	C88_ - KeyHook_start
	.dl	C89_ - KeyHook_start
    
#define AMOUNT_OF_CUSTOM_TOKENS 6
Token1: .db 8,  "ExecHex(", 0
Token2: .db 13, "DefineSprite(", 0
Token3: .db 5,  "Call ", 0
Token4: .db 12, "CompilePrgm(", 0
Token5: .db 12, "SetBASICVar(", 0
Token6: .db 12, "GetBASICVar(", 0

CustomTokensProgramText:
	.db	"PROGRAM:", 0
KeyHook_end:

TokenHook_start:
	.db	83h
	ld	a, d
	cp	a, 4
	ret	nz
	ld	a, e
	cp	a, 5+3+(AMOUNT_OF_CUSTOM_TOKENS*3)
	ret	nc
	sub	a, 5
	ld	de, (rawKeyHookPtr)
	ld	hl, TokenHook_data - KeyHook_start
	add	hl, de
	ld	bc, 0
	ld	c, a
	add	hl, bc
	ld	hl, (hl)
	add	hl, de
	ret
TokenHook_data:
	.dl	Token1 - KeyHook_start - 1
	.dl	Token2 - KeyHook_start - 1
	.dl	Token3 - KeyHook_start - 1
	.dl	Token4 - KeyHook_start - 1
	.dl	Token5 - KeyHook_start - 1
	.dl	Token6 - KeyHook_start - 1
TokenHook_end:

CursorHook_start:
	.db	83h
	cp	a, 24h
	jr	nz, +_
	inc	a
	ld	a, (curUnder)
	ret
_:	cp	a, 22h
	ret	nz
	ld	a, (cxCurApp)
	cp	a, cxPrgmEdit
	ret	nz
	ld	hl, (editCursor)
	ld	a, (hl)
	cp	a, tDet
	ret	nz
DrawDetText:
	bit	displayed_det, (iy+fAlways1)
	ret	nz
	ld	hl, (editTail)
	inc	hl
	ld	a, (hl)
	sub	a, t0
	ret	c
	cp	a, t9-t0+1
	ld	bc, (editBtm)
	ld	de, 0
	ld	e, a
	jr	c, GetDetValueLoop
WrongDetValue:
	inc	a
	ret
GetDetValueLoop:
	inc	hl
	or	a, a
	sbc	hl, bc
	jr	z, GetDetValueStop
	add	hl, bc
	ld	a, (hl)
	sub	a, t0
	jr	c, GetDetValueStop
	cp	a, t9-t0+1
	jr	nc, GetDetValueStop
	push	hl
	ex	de, hl
	add	hl, hl
	push	hl
	pop	de
	add	hl, hl
	add	hl, hl
	add	hl, de
	ld	de, 0
	ld	e, a
	add	hl, de
	ex	de, hl
	pop	hl
	jr	GetDetValueLoop
GetDetValueStop:
	ex	de, hl
	ld	de, AMOUNT_OF_C_FUNCTIONS
	or	a, a
	sbc	hl, de
	jr	nc, WrongDetValue
	add	hl, de
	ld	h, 3
	mlt	hl
	ld	de, CData5 - KeyHook_start
	add	hl, de
	ld	de, (rawKeyHookPtr)
	add	hl, de
	ld	hl, (hl)
	add	hl, de
	ld	de, 000E71Ch
	ld.sis	(drawFGColor & 0FFFFh), de
	ld.sis	de, (statusBarBGColor & 0FFFFh)
	ld.sis	(drawBGColor & 0FFFFh), de
	ld	a, 14
	ld	(penRow),a
	ld	de, 2
	ld.sis	(penCol & 0FFFFh), de
	call	_VPutS
	ld	de, 0FFFFh
	ld.sis	(drawBGColor & 0FFFFh), de
	set	displayed_det, (iy+fAlways1)
	ret
CursorHook_end:

Hooks_end: