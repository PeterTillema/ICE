InsertA:
	ld hl, (programPtr)
	ld (hl), a
	inc hl
	ld (programPtr), hl
	ret

InsertHL:
	push de
		ex de,hl
		ld hl, (programPtr)
		ld (hl), de
		inc hl
		inc hl
		inc hl
		ld (programPtr), hl
	pop de
	ret
	
InsertAIXC:
	ld a, 079h
	jr +_
InsertAIXE:
	ld a, 07Bh
_:	ld (SMC2), a
	ld a, 07Eh
	ld (SMC1), a
	jr Insert
InsertLIXC:
	ld a, 079h
	jr +_
InsertLIXE:
	ld a, 07Bh
_:	ld (SMC2), a
	ld a, 06Eh
	ld (SMC1), a
Insert:
	ld a, 0DDh
	call InsertA
SMC1 = $+1
	ld a, 0
	call InsertA
SMC2:
	.db 0
	jp InsertA
	
InsertProgramPtrToDataOffset:
	ld hl, (programDataOffsetPtr)
	ld bc, (programPtr)
	ld (hl), bc
	inc hl
	inc hl
	inc hl
	ld (programDataOffsetPtr), hl
	ret

EndError:
	ld hl, EndErrorMessage
DisplayErrorMessage:
	push hl
		call _ClrLCDFull
		call _DrawStatusBar
		call _HomeUp
	pop hl
	jp _PutS
	
GetFunction:
	cp tGetKey
	jr nz, +_
	ld a, 0CDh
	call InsertA							; call ******
	ld hl, _GetCSC
	jp InsertHL								; call _GetCSC
_:	cp trand
	ret nz
	ld hl, 04F5FEDh
	call InsertHL							; ld a, r \ ld c, a
	ld hl, 0878187h
	call InsertHL							; add a, a \ add a, c \ add a, a
	ld a, 087h
	call InsertA							; add a, a
	ld a, 081h
	jp InsertA								; add a, c
	
CompareStrings:
	ld a, (de)
	cp a, (hl)
	ret nz
	inc hl
	inc de
	cp tEnter
	jr nz,CompareStrings
	ret
