functionAdd:
	ld a, b
	rra				; b contain odd or even value? (test bit 0)
	jp nc, AddVariableXXX		; if even, means b=4
	rra				; test bit 1
	jp c, AddFunctionXXX		; if set, means b=3
	rra				; test bit 2
	jp nc, AddNumberXXX		; if reset, means b=1
AddChainXXX:
	bit 0, c
	jr nz, AddChainFirstXXX
AddChainPushXXX:
	ld a, d
	rra
	jr nc, AddChainPushVariable
	rra
	jr c, AddChainPushFunction
	rra
	jr nc, AddChainPushNumber
AddChainPushChain:
	ld a, 0C1h
	call InsertA						; pop bc
	ld a, 080h
	jp InsertA							; add a, b
AddChainPushNumber:
	ld a, 0F1h
	call InsertA						; pop af
	jr AddChainFirstNumber
AddChainPushVariable:
	ld a, 0F1h
	call InsertA						; pop af
	jr AddChainFirstVariable
AddChainPushFunction:
	ld a, 0C1h
	call InsertA						; pop bc
	ld a, e
	call GetFunction
	ld a, 080h
	jp InsertA							; add a, b
AddChainFirstXXX:
	ld a, d
	rra
	jr nc,AddChainFirstVariable
	rra
	jr c, AddChainFirstFunction
AddChainFirstNumber:
	ld a, e
	dec a
	ret c
	jr nz, +_
	ld a, 03Ch
	jp InsertA							; inc a
_:	cp 254
	jr nz, +_
	ld a, 03Dh
	jp InsertA							; dec a
_:	ld a, 0C6h
	call InsertA						; add a, **
	ld a, e
	jp InsertA							; add a, XX
AddChainFirstVariable:
	ld a, 0DDh
	call InsertA						; add a, (ix+*) (1)
	ld a, 086h
	call InsertA						; add a, (ix+*) (2)
	ld a, e
	jp InsertA							; add a, (ix+*) (3)
AddNumberXXX:
	ld a, d
	rra
	jr nc, AddNumberVariable
	rra
	jr c, AddNumberFunction
	rra
	jr nc, AddNumberNumber
AddNumberChain:
	ld e, c
	jr AddChainFirstNumber
AddNumberNumber:
	ld a, c
	add a, e
	inc hl
	ld (hl), a
	res chain_operators, (iy+myFlags)
	ret
AddNumberVariable:
	call InsertAIXE						; ld a, (ix+*)
	ld e, c
	jr AddChainFirstNumber
AddNumberFunction:
	ld a, e
	call GetFunction
	ld e, c
	jr AddChainFirstNumber
AddVariableXXX:
	ld a, d
	rra
	jr nc, AddVariableVariable
	rra
	jr c, AddVariableFunction
	rra
	jr nc, AddVariableNumber
AddVariableChain:
	ld e, c
	jp AddChainFirstVariable
AddVariableNumber:
	ld a, c
	ld c, e
	ld e, a
	jr AddNumberVariable
AddVariableVariable:
	call InsertAIXC						; ld a, (ix+*)
	ld a, c
	cp e
	jp nz, AddChainFirstVariable
	ld a, 087h
	jp InsertA							; add a, a
AddVariableFunction:
	ld a, e
	call GetFunction
	ld e, c
	jp AddChainFirstVariable
AddFunctionXXX:
	ld a, c
	ld c, e
	ld e, a
	ld a, d
	rra
	jr nc, AddVariableFunction
	rra
	jr c,AddFunctionFunction
	rra
	jr nc, AddNumberFunction
AddChainFirstFunction:
	ld a, 047h
	call InsertA						; ld b, a
	ld a, e
	call GetFunction
	ld a, 080h
	jp InsertA							; add a, b
AddFunctionFunction:
	ld a, c
	call GetFunction
	ld a, 047h
	call InsertA						; ld b, a
	ld a, e
	call GetFunction
	ld a, 080h
	jp InsertA							; add a, b
