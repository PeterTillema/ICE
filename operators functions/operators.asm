ExecuteOperator:
	ld	(tempToken2), a
	ld	hl, ExprOutput2
	ld	(hl), OutputIsInHL
	or	a, a
	jp	z, ListElementXXX
	cp	tLE
	call	z, SwapLEToGE
	cp	tLT
	call	z, SwapLTToGT
	ld	(tempToken), a
	push	bc
	ld	hl, operators_booleans
	ld	bc, 14
	cpir
	ld	hl, operators_special
	add	hl, bc
	ld	c, (hl)
	ld	a, c
	cp	4
	jr	nz, +_
	set	ans_set_z_flag, (iy+fExpression1)
_:	ld	b, 18
	mlt	bc
	ld	hl, operator_start	; start of bunch jumps
	add	hl, bc			; start of right operator
	ld	a, (ix-8)
	ld	c, a
	cp	6
	jp	nc, ErrorSyntax
	ld	b, 3
	mlt	bc
	add	hl, bc			; start of right subelement
	ld	hl, (hl)
	pop	bc
	jp	(hl)
	
ListElementXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, ListElementNumber
	dec	a
	jr	z, ListElementVariable
	dec	a
	jr	z, ListElementChainPush
	dec	a
	jr	z, ListElementChainAns
	dec	a
	jr	z, ListElementFunction
ListElementNumber:
	ld	a, 02Ah
	push	bc
	pop	hl
	add	hl, de
	add	hl, de
	add	hl, de
	jp	InsertAHL		; ld hl, (*)
ListElementVariable:
	call	InsertIXE		; ld de, (ix+*)
	jr	+_
ListElementChainPush:
	jp	UnknownError
ListElementChainAns:
	call	MaybeChangeHLToDE
_:	ld	a, 021h
	push	bc
	pop	de
	ld	hl, 0191919h
	call	InsertADEHL		; ld hl, * \ add hl, de \ add hl, de \ add hl, de
	ld	a, 0EDh
	call	InsertA			; ld hl, (hl) (1)
	ld	a, 027h
	jp	InsertA			; ld hl, (hl) (2)
ListElementFunction:
	ld	a, e
	ld	b, OutputInDE
	call	GetFunction
	jr	-_
	
SubNumberXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, SubNumberNumber
	dec	a
	jr	z, SubNumberVariable
	dec	a
	jr	z, SubNumberChainPush
	dec	a
	jr	z, SubNumberChainAns
	dec	a
	jr	z, SubNumberFunction
	jp	SubError
SubNumberNumber:
	set	output_is_number, (iy+fExpression1)
	push	bc
	pop	hl
	or	a, a
	sbc	hl, de
	ld	(ix-7), hl
	ret
SubNumberVariable:
	ld	a, 021h
	push	bc
	pop	hl
	call	InsertAHL		; ld hl, *
	jp	SubChainAnsVariable
SubNumberChainPush:
	jp	UnknownError
SubNumberChainAns:
	call	MaybeChangeHLToDE
	ld	a, 021h
	push	bc
	pop	de
	ld	hl, 052EDB7h
	jp	InsertADEHL		; ld hl, * \ or a \ sbc hl, de
SubNumberFunction:
	ld	a, e
	push	bc
	ld	b, OutputInDE
	call	GetFunction
	ld	a, 021h
	pop	de
	ld	hl, 052EDB7h
	jp	InsertADEHL		; ld hl, * \ or a \ sbc hl, de
SubVariableXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, SubVariableNumber
	dec	a
	jr	z, SubVariableVariable
	dec	a
	jr	z, SubVariableChainPush
	dec	a
	jr	z, SubVariableChainAns
	dec	a
	jp	z, SubVariableFunction
	jp	SubError
SubVariableNumber:
	call	InsertHIXC		; ld hl, (ix+*)
	jp	SubChainAnsNumber
SubVariableVariable:
	ld	a, c
	cp	e
	jr	nz, +_
	set	output_is_number, (iy+fExpression1)
	ld	(hl), typeNumber
	ld	de, 0
	inc	hl
	ld	(hl), de
	ret
_:	call	InsertHIXC		; ld hl, (ix+*)
	jp	SubChainAnsVariable
SubVariableChainPush:
	jp	UnknownError
SubVariableChainAns:
	call	MaybeChangeHLToDE
	call	InsertHIXC		; ld hl, (ix+*)
	ld	hl, 052EDB7h
	jp	InsertHL		; or a \ sbc hl, de
SubVariableFunction:
	ld	a, e
	ld	b, OutputInDE
	call	GetFunction
	call	InsertHIXC		; ld hl, (ix+*)
	ld	hl, 052EDB7h
	jp	InsertHL		; or a \ sbc hl, de
SubChainPushXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, SubChainPushNumber
	dec	a
	jr	z, SubChainPushVariable
	dec	a
	jr	z, SubChainPushChainPush
	dec	a
	jr	z, SubChainPushChainAns
	dec	a
	jp	z, SubChainPushFunction
	jp	SubError
SubChainPushNumber:
	ld	a, 0E1h
	call	InsertA			; pop hl
	jr	SubChainAnsNumber
SubChainPushVariable:
	ld	a, 0E1h
	call	InsertA			; pop hl
	jr	SubChainAnsVariable
SubChainPushChainPush:
	jp	UnknownError
SubChainPushChainAns:
	call	MaybeChangeHLToDE
	ld	a, 0E1h
	ld	hl, 052EDB7h
	jp	InsertAHL		; pop hl \ or a \ sbc hl, de
SubChainPushFunction:
	ld	a, 0E1h
	call	InsertA			; pop hl
	jr	SubChainAnsFunction
SubChainAnsXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, SubChainAnsNumber
	dec	a
	jr	z, SubChainAnsVariable
	dec	a
	jr	z, SubChainAnsChainPush
	dec	a
	jr	z, SubChainAnsChainAns
	dec	a
	jp	z, SubChainAnsFunction
	jp	SubError
SubChainAnsNumber:
	or	a, a
	sbc	hl, hl
	ex	de, hl
	sbc	hl, de
	ret	z
	ld	e, 7
	sbc	hl, de
	jr	nc, SubHLDE
	add	hl, de
	ld	b, l
	ld	a, (ExprOutput)
	or	a, a
	jr	z, ++_
	ld	a, 02Bh
_:	call	InsertA			; dec hl/de
	djnz	-_
	ret
_:	ld	a, OutputIsInDE
	ld	(ExprOutput2), a
	ld	a, 01Bh
	jr	--_
SubHLDE:
	add	hl, de
	call	__ineg
	ld	a, (ExprOutput)
	or	a, a
	ld	a, 011h
	jr	nz, +_
	ld	a, 021h
_:	call	InsertAHL		; ld hl/de, 16777215-*
	ld	a, 019h
	jp	InsertA			; add hl, de
SubChainAnsVariable:
	call	MaybeChangeDEToHL
	call	InsertIXE		; ld de, (ix+*)
	ld	hl, 052EDB7h
	jp	InsertHL		; or a \ sbc hl, de
SubChainAnsChainPush:
	jp	UnknownError
SubChainAnsChainAns:
	jp	UnknownError
SubChainAnsFunction:
	ld	a, (ExprOutput)
	add	a, a
	add	a, a
	add	a, a
	add	a, a
	add	a, 0D5h
	call	InsertA			; push hl/de
	ld	a, e
	ld	b, OutputInDE
	call	GetFunction
	ld	a, 0E1h
	ld	hl, 052EDB7h
	jp	InsertAHL		; pop hl \ or a \ sbc hl, de
SubFunctionXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, SubFunctionNumber
	dec	a
	jr	z, SubFunctionVariable
	dec	a
	jr	z, SubFunctionChainPush
	dec	a
	jr	z, SubFunctionChainAns
	dec	a
	jr	z, SubFunctionFunction
	jp	SubError
SubFunctionNumber:
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jp	SubChainAnsNumber
SubFunctionVariable:
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jp	SubChainAnsVariable
SubFunctionChainPush:
	jp	UnknownError
SubFunctionChainAns:
	ld	a, (ExprOutput)
	add	a, a
	add	a, a
	add	a, a
	add	a, a
	add	a, 0D5h
	call	InsertA			; push hl/de
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	ld	a, 0E1h
	ld	hl, 052EDB7h
	jp	InsertAHL		; pop de \ or a \ sbc hl, de
SubFunctionFunction:
	ld	a, e
	ld	b, OutputInHL
	call	GetFunction
	ld	a, c
	ld	b, OutputInDE
	set	need_push, (iy+fExpression1)
	call	GetFunction
	ld	hl, 052EDB7h
	jp	InsertHL		; or a \ sbc hl, de

AddNumberXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, AddNumberNumber
	dec	a
	jr	z, AddNumberVariable
	dec	a
	jr	z, AddNumberChainPush
	dec	a
	jr	z, AddNumberChainAns
	dec	a
	jr	z, AddNumberFunction
	jp	AddError
AddNumberNumber:
	set	output_is_number, (iy+fExpression1)
	ex	de, hl
	add	hl, bc
	ld	(ix-7), hl
	ret
AddNumberVariable:
	push	bc
	pop	hl
	ex	de, hl
	push	hl
	pop	bc
	jr	AddVariableNumber
AddNumberChainPush:
	jp	UnknownError
AddNumberChainAns:
	push	bc
	pop	de
	jp	AddChainAnsNumber
AddNumberFunction:
	push	bc
	pop	hl
	ex	de, hl
	push	hl
	pop	bc
	jp	AddFunctionNumber
AddVariableXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, AddVariableNumber
	dec	a
	jr	z, AddVariableVariable
	dec	a
	jr	z, AddVariableChainPush
	dec	a
	jr	z, AddVariableChainAns
	dec	a
	jp	z, AddVariableFunction
	jp	AddError
AddVariableNumber:
	call	InsertHIXC		; ld hl, (ix+*)
	jp	AddChainAnsNumber
AddVariableVariable:
	call	InsertHIXC		; ld hl, (ix+*)
	jp	AddChainAnsVariable
AddVariableChainPush:
	ld	a, 0E1h
	call	InsertA			; pop hl
	ld	e, c
	jp	AddChainAnsVariable
AddVariableChainAns:
	ld	e, c
	jp	AddChainAnsVariable
AddVariableFunction:
	ld	a, e
	ld	b, OutputInHL
	call	GetFunction
	ld	e, c
	call	InsertIXE		; ld de, (ix+*)
	ld	a, 019h
	jp	InsertA			; add hl, de
AddChainPushXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, AddChainPushNumber
	dec	a
	jr	z, AddChainPushVariable
	dec	a
	jr	z, AddChainPushChainPush
	dec	a
	jr	z, AddChainPushChainAns
	dec	a
	jp	z, AddChainPushFunction
	jp	AddError
AddChainPushNumber:
	ld	a, 0E1h
	call	InsertA			; pop hl
	jr	AddChainAnsNumber
AddChainPushVariable:
	ld	a, 0E1h
	call	InsertA			; pop hl
	jr	AddChainAnsVariable
AddChainPushChainPush:
	jp	UnknownError
AddChainPushChainAns:
	ld	a, (ExprOutput)
	or	a, a
	ld	a, 0D1h
	jr	nz, +_
	ld	a, 0E1h
_:	call	InsertA			; pop hl/de
	ld	a, 019h
	jp	InsertA			; add hl, de
AddChainPushFunction:
	ld	a, 0E1h
	call	InsertA			; pop hl
	jr	AddChainAnsFunction
AddChainAnsXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, AddChainAnsNumber
	dec	a
	jr	z, AddChainAnsVariable
	dec	a
	jr	z, AddChainAnsChainPush
	dec	a
	jr	z, AddChainAnsChainAns
	dec	a
	jp	z, AddChainAnsFunction
	jp	AddError
AddChainAnsNumber:
	or	a, a
	sbc	hl, hl
	ex	de, hl
	sbc	hl, de
	ret	z
	ld	e, 6
	sbc	hl, de
	jr	nc, AddHLDE
	add	hl, de
	ld	b, l
	ld	a, (ExprOutput)
	or	a, a
	jr	z, ++_
	ld	a, 023h
_:	call	InsertA			; inc hl/de
	djnz	-_
	ret
_:	ld	a, OutputIsInDE
	ld	(ExprOutput2), a
	ld	a, 013h
	jr	--_
AddHLDE:
	add	hl, de
	ld	a, (ExprOutput)
	or	a, a
	ld	a, 011h
	jr	nz, +_
	ld	a, 021h
_:	call	InsertAHL		; ld hl/de, *
	ld	a, 019h
	jp	InsertA			; add hl, de
AddChainAnsVariable:
	ld	h, 017h
	ld	d, 3
	mlt	de
	ld	a, e
	ld	l, 0DDh
	call	_SetHLUToA
	ld	a, (ExprOutput)
	or	a, a
	jr	nz, +_
	ld	h, 027h
_:	call	InsertHL		; ld hl/de, (ix+*)
	ld	a, 019h
	jp	InsertA			; add hl, de
AddChainAnsChainPush:
	jp	UnknownError
AddChainAnsChainAns:
	jp	UnknownError
AddChainAnsFunction:
	ld	a, (ExprOutput)
	add	a, a
	add	a, a
	add	a, a
	add	a, a
	add	a, 0D5h
	call	InsertA			; push hl/de
	ld	a, e
	ld	b, OutputInHL
	call	GetFunction
	ld	a, 0D1h
	call	InsertA			; pop de
	ld	a, 019h
	jp	InsertA			; add hl, de
AddFunctionXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, AddFunctionNumber
	dec	a
	jr	z, AddFunctionVariable
	dec	a
	jr	z, AddFunctionChainPush
	dec	a
	jr	z, AddFunctionChainAns
	dec	a
	jr	z, AddFunctionFunction
	jp	AddError
AddFunctionNumber:
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jp	AddChainAnsNumber
AddFunctionVariable:
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jp	AddChainAnsVariable
AddFunctionChainPush:
	jp	UnknownError
AddFunctionChainAns:
	ld	a, (ExprOutput)
	add	a, a
	add	a, a
	add	a, a
	add	a, a
	add	a, 0D5h
	call	InsertA			; push hl/de
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	ld	a, 0D1h
	call	InsertA			; pop de
	ld	a, 019h
	jp	InsertA			; add hl, de
AddFunctionFunction:
	ld	a, e
	ld	b, OutputInHL
	call	GetFunction
	ld	a, c
	ld	b, OutputInDE
	set	need_push, (iy+fExpression1)
	call	GetFunction
	ld	a, 019h
	jp	InsertA			; add hl, de
	
DivNumberXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, DivNumberNumber
	dec	a
	jr	z, DivNumberVariable
	dec	a
	jr	z, DivNumberChainPush
	dec	a
	jr	z, DivNumberChainAns
	dec	a
	jr	z, DivNumberFunction
	jp	DivError
DivNumberNumber:
	set	output_is_number, (iy+fExpression1)
	push	bc
	pop	hl
	push	de
	pop	bc
	call	__idvrmu
	ld	(ix-7), de
	ret
DivNumberVariable:
	ld	a, 021h
	push	bc
	pop	hl
	call	InsertAHL		; ld hl, *
	jp	DivChainAnsVariable
DivNumberChainPush:
	jp	UnknownError
DivNumberChainAns:
	ld	de, 021C1E5h
	push	bc
	pop	hl
	ld	a, (ExprOutput)
	or	a, a
	jr	nz, +_
	ld	e, 0D5h
_:	call	InsertDEHL		; push hl/de \ pop bc \ ld hl, *
	jr	DivInsert
DivNumberFunction:
	ld	a, e
	push	bc
	ld	b, OutputInBC
	call	GetFunction
	ld	a, 021h
	pop	hl
	call	InsertAHL		; ld hl, *
	jr	DivInsert
DivVariableXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, DivVariableNumber
	dec	a
	jr	z, DivVariableVariable
	dec	a
	jr	z, DivVariableChainPush
	dec	a
	jr	z, DivVariableChainAns
	dec	a
	jp	z, DivVariableFunction
	jp	DivError
DivVariableNumber:
	call	InsertHIXC		; ld hl, (ix+*)
	jp	DivChainAnsNumber
DivVariableVariable:
	ld	a, c
	cp	e
	jr	nz, +_
	set	output_is_number, (iy+fExpression1)
	ld	(hl), typeNumber
	ld	de, 1
	inc	hl
	ld	(hl), de
	ret
_:	call	InsertHIXC		; ld hl, (ix+*)
	jp	DivChainAnsVariable
DivVariableChainPush:
	jp	UnknownError
DivVariableChainAns:
	ld	a, (ExprOutput)
	add	a, a
	add	a, a
	add	a, a
	add	a, a
	add	a, 0D5h
	call	InsertA			; push hl/de
	ld	a, 0C1h
	call	InsertA			; pop bc
	call	InsertHIXC		; ld hl, (ix+*)
	jr	DivInsert
DivVariableFunction:
	ld	a, e
	ld	b, OutputInBC
	call	GetFunction
	call	InsertHIXC		; ld hl, (ix+*)
DivInsert:
	ld	a, OutputIsInDE
	ld	(ExprOutput2), a
	ld	a, 0CDh
	ld	hl, __idvrmu
	jp	InsertAHL		; call __idvrmu
DivChainPushXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, DivChainPushNumber
	dec	a
	jr	z, DivChainPushVariable
	dec	a
	jr	z, DivChainPushChainPush
	dec	a
	jr	z, DivChainPushChainAns
	dec	a
	jp	z, DivChainPushFunction
	jp	DivError
DivChainPushNumber:
	ld	a, 0E1h
	call	InsertA			; pop hl
	jr	DivChainAnsNumber
DivChainPushVariable:
	ld	a, 0E1h
	call	InsertA			; pop hl
	jr	DivChainAnsVariable
DivChainPushChainPush:
	jp	UnknownError
DivChainPushChainAns:
	ld	hl, 0E1C1E5h
	ld	a, (ExprOutput)
	add	a, a
	add	a, a
	add	a, a
	add	a, a
	add	a, 0D5h
	ld	l, a
	call	InsertHL		; push hl/de \ pop bc \ pop hl
_:	jr	DivInsert
DivChainPushFunction:
	ld	a, 0E1h
	call	InsertA			; pop hl
	jr	DivChainAnsFunction
DivChainAnsXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, DivChainAnsNumber
	dec	a
	jr	z, DivChainAnsVariable
	dec	a
	jr	z, DivChainAnsChainPush
	dec	a
	jr	z, DivChainAnsChainAns
	dec	a
	jp	z, DivChainAnsFunction
	jp	DivError
DivChainAnsNumber:
	call	MaybeChangeDEToHL
	ld	a, 001h
	ex	de, hl
	ld	de, 256
	or	a, a
	sbc	hl, de
	jr	nc, +_
	add	hl, de
	ld	c, l
	ld	a, 03Eh
	call	InsertA			; ld a, *
	ld	a, c
	call	InsertA			; ld a, *
	ld	a, 0CDh
	ld	hl, _DivHLByA
	jp	InsertAHL		; call _DivHLByA
_:	add	hl, de
	call	InsertAHL		; ld bc, *
_:	jr	---_
DivChainAnsVariable:
	call	MaybeChangeDEToHL
	call	InsertIXC
	jr	-_
DivChainAnsChainPush:
	jp	UnknownError
DivChainAnsChainAns:
	jp	UnknownError
DivChainAnsFunction:
	ld	a, (ExprOutput)
	add	a, a
	add	a, a
	add	a, a
	add	a, a
	add	a, 0D5h
	call	InsertA			; push hl/de
	ld	a, e
	ld	b, OutputInBC
	call	GetFunction
	ld	a, 0E1h
	call	InsertA			; pop hl
_:	jr	--_
DivFunctionXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, DivFunctionNumber
	dec	a
	jr	z, DivFunctionVariable
	dec	a
	jr	z, DivFunctionChainPush
	dec	a
	jr	z, DivFunctionChainAns
	dec	a
	jr	z, DivFunctionFunction
	jp	DivError
DivFunctionNumber:
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jp	DivChainAnsNumber
DivFunctionVariable:
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jp	DivChainAnsVariable
DivFunctionChainPush:
	jp	UnknownError
DivFunctionChainAns:
	ld	a, (ExprOutput)
	add	a, a
	add	a, a
	add	a, a
	add	a, a
	add	a, 0D5h
	call	InsertA			; push hl/de
	ld	a, 0C1h
	call	InsertA			; pop bc
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jr	-_
DivFunctionFunction:
	ld	a, e
	ld	b, OutputInBC
	call	GetFunction
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jr	-_
	
MulNumberXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, MulNumberNumber
	dec	a
	jr	z, MulNumberVariable
	dec	a
	jr	z, MulNumberChainPush
	dec	a
	jr	z, MulNumberChainAns
	dec	a
	jr	z, MulNumberFunction
	jp	MulError
MulNumberNumber:
	set	output_is_number, (iy+fExpression1)
	ex	de, hl
	call	__imulu
	ld	(ix-7), hl
	ret
MulNumberVariable:
	push	bc
	pop	hl
	ex	de, hl
	push	hl
	pop	bc
	jr	MulVariableNumber
MulNumberChainPush:
	jp	UnknownError
MulNumberChainAns:
	push	bc
	pop	de
	jp	MulChainAnsNumber
MulNumberFunction:
	push	bc
	pop	hl
	ex	de, hl
	push	hl
	pop	bc
	jp	MulFunctionNumber
MulVariableXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, MulVariableNumber
	dec	a
	jr	z, MulVariableVariable
	dec	a
	jr	z, MulVariableChainPush
	dec	a
	jr	z, MulVariableChainAns
	dec	a
	jp	z, MulVariableFunction
	jp	MulError
MulVariableNumber:
	call	InsertHIXC		; ld hl, (ix+*)
	jp	MulChainAnsNumber
MulVariableVariable:
	call	InsertHIXC		; ld hl, (ix+*)
	jp	MulChainAnsVariable
MulVariableChainPush:
	ld	a, 0E1h
	call	InsertA			; pop hl
	ld	e, c
	jp	MulChainAnsVariable
MulVariableChainAns:
	ld	e, c
	jp	MulChainAnsVariable
MulVariableFunction:
	ld	a, e
	ld	b, OutputInHL
	call	GetFunction
	call	InsertIXC
	ld	hl, __imulu
	jp	InsertCallHL		; call __imulu
MulChainPushXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, MulChainPushNumber
	dec	a
	jr	z, MulChainPushVariable
	dec	a
	jr	z, MulChainPushChainPush
	dec	a
	jr	z, MulChainPushChainAns
	dec	a
	jp	z, MulChainPushFunction
	jp	MulError
MulChainPushNumber:
	ld	a, 0E1h
	call	InsertA			; pop hl
	jr	MulChainAnsNumber
MulChainPushVariable:
	ld	a, 0E1h
	call	InsertA			; pop hl
	jp	MulChainAnsVariable
MulChainPushChainPush:
	jp	UnknownError
MulChainPushChainAns:
	call	MaybeChangeDEToHL
	ld	a, 0C1h
	call	InsertA			; pop bc
	ld	a, 0CDh
	ld	hl, __imulu
	jp	InsertAHL		; call __imulu
MulChainPushFunction:
	ld	a, 0E1h
	call	InsertA			; pop hl
	jp	MulChainAnsFunction
MulChainAnsXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, MulChainAnsNumber
	dec	a
	jp	z, MulChainAnsVariable
	dec	a
	jp	z, MulChainAnsChainPush
	dec	a
	jp	z, MulChainAnsChainAns
	dec	a
	jp	z, MulChainAnsFunction
	jp	MulError
MulChainAnsNumber:
	call	_ChkDEIs0
	jr	nz, +_
	ld	hl, 062EDB7h
	jp	InsertHL
_:	push	de
	pop	hl
	ld	b, 0
	ld	a, 26
_:	dec	a
	add	hl, hl
	jr	nc, -_
_:	adc	a, b
	adc	hl, hl
	jr	nz, -_
	rl	b
	jr	nz, +_
	sub	a, 3
_:	cp	a, 10
	jr	c, ++_
	call	MaybeChangeDEToHL
	ld	hl, -256
	add	hl, de
	jr	c, +_
	ld	hl, 0CD003Eh
	ld	h, e
	ld	de, __imul_b
	ex	de, hl
	jp	InsertDEHL
_:	ld	a, 001h
	ex	de, hl
	call	InsertAHL
	ld	hl, __imulu
	jp	InsertCallHL
_:	djnz	+_
	ld	a, (ExprOutput)
	rrca
	ccf
	sbc	a, a
	and	a, 0D5h - 0E5h
	add	a, 0E5h
	call	InsertA
	xor	a, 0E5h ^ 0D1h		; == 0D5h ^ 0E1h
	call	InsertA
_:	ex	de, hl
	scf
_:	adc	hl, hl
	jr	nc, -_
_:	or	a, a
	adc	hl, hl
	ret	z
	ld	a, 029h
	call	InsertA
	ld	a, 019h
	call	c, InsertA
	jr	-_
MulChainAnsVariable:
	call	MaybeChangeDEToHL
	call	InsertIXC		; ld bc, (ix+*)
	ld	a, 0CDh
	ld	hl, __imulu
	jp	InsertAHL		; call __imulu
MulChainAnsChainPush:
	jp	UnknownError
MulChainAnsChainAns:
	jp	UnknownError
MulChainAnsFunction:
	ld	a, (ExprOutput)
	add	a, a
	add	a, a
	add	a, a
	add	a, a
	add	a, 0D5h
	call	InsertA			; push hl/de
	ld	a, e
	ld	b, OutputInBC
	call	GetFunction
	ld	a, 0E1h
	call	InsertA			; pop hl
	ld	a, 0CDh
	ld	hl, __imulu
	jp	InsertAHL		; call __imulu
MulFunctionXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, MulFunctionNumber
	dec	a
	jr	z, MulFunctionVariable
	dec	a
	jr	z, MulFunctionChainPush
	dec	a
	jr	z, MulFunctionChainAns
	dec	a
	jr	z, MulFunctionFunction
	jp	MulError
MulFunctionNumber:
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jp	MulChainAnsNumber
MulFunctionVariable:
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jp	MulChainAnsVariable
MulFunctionChainPush:
	jp	UnknownError
MulFunctionChainAns:
	ld	e, c
	jr	MulChainAnsFunction
MulFunctionFunction:
	ld	a, e
	ld	b, OutputInBC
	call	GetFunction
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	ld	a, 0CDh
	ld	hl, __imulu
	jp	InsertAHL		; call __imulu

SwapLEToGE:
	ld	a, tGE
	jr	$+4	
SwapLTToGT:
	ld	a, tGT
	push	bc
	pop	hl
	ld	c, (ix-4)
	ld	b, (ix-8)
	ld	(ix-4), b
	ld	(ix-8), c
	ex	de, hl
	push	hl
	pop	bc
	ret

GLETNumberXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, GLETNumberNumber
	dec	a
	jr	z, GLETNumberVariable
	dec	a
	jr	z, GLETNumberChainPush
	dec	a
	jr	z, GLETNumberChainAns
	dec	a
	jr	z, GLETNumberFunction
	jp	GLETError
GLETNumberNumber:
	set	output_is_number, (iy+fExpression1)
	push	bc
	pop	hl
	ld	a, (tempToken)
	cp	tGE
	jr	z, +_
	scf
_:	sbc	hl, de
	sbc	hl, hl
	inc	hl
	ld	(ix-7), hl
	ret
GLETNumberVariable:
	ld	a, 021h
	push	bc
	pop	hl
	call	InsertAHL		; ld hl, *
	jp	GLETChainAnsVariable
GLETNumberChainPush:
	jp	UnknownError
GLETNumberChainAns:
	call	MaybeChangeHLToDE
	ld	a, 021h
	push	bc
	pop	hl
	call	InsertAHL		; ld hl, *
	jr	GLETInsert
GLETNumberFunction:
	ld	a, e
	push	bc
	ld	b, OutputInDE
	call	GetFunction
	ld	a, 021h
	pop	hl
	call	InsertAHL		; ld hl, *
	jr	GLETInsert
GLETVariableXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, GLETVariableNumber
	dec	a
	jr	z, GLETVariableVariable
	dec	a
	jr	z, GLETVariableChainPush
	dec	a
	jr	z, GLETVariableChainAns
	dec	a
	jp	z, GLETVariableFunction
	jp	GLETError
GLETVariableNumber:
	call	InsertHIXC		; ld hl, (ix+*)
	jp	GLETChainAnsNumber
GLETVariableVariable:
	ld	a, c
	cp	e
	jr	nz, ++_
	set	output_is_number, (iy+fExpression1)
	ld	a, (tempToken)
	sub	tGT
	jr	z, +_
	ld	a, 1
_:	ld	de, 0
	ld	e, a
	ld	(hl), typeNumber
	inc	hl
	ld	(hl), de
	ret
_:	call	InsertHIXC		; ld hl, (ix+*)
	jp	GLETChainAnsVariable
GLETVariableChainPush:
	jp	UnknownError
GLETVariableChainAns:
	call	MaybeChangeHLToDE
	call	InsertHIXC		; ld hl, (ix+*)
	jr	GLETInsert
GLETVariableFunction:
	ld	a, e
	ld	b, OutputInDE
	call	GetFunction
	call	InsertHIXC		; ld hl, (ix+*)
GLETInsert:
	ld	de, 052ED37h
	ld	hl, 02362EDh
	ld	a, (tempToken)
	cp	tGT
	jr	z, +_
	ld	e, 0B7h
_:	jp	InsertDEHL		; scf/or a \ sbc hl, de \ sbc hl, hl \ inc hl
GLETChainPushXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, GLETChainPushNumber
	dec	a
	jr	z, GLETChainPushVariable
	dec	a
	jr	z, GLETChainPushChainPush
	dec	a
	jr	z, GLETChainPushChainAns
	dec	a
	jp	z, GLETChainPushFunction
	jp	GLETError
GLETChainPushNumber:
	ld	a, 0E1h
	call	InsertA			; pop hl
	jr	GLETChainAnsNumber
GLETChainPushVariable:
	ld	a, 0E1h
	call	InsertA			; pop hl
	jr	GLETChainAnsVariable
GLETChainPushChainPush:
	jp	UnknownError
GLETChainPushChainAns:
	call	MaybeChangeHLToDE
	ld	a, 0E1h
	call	InsertA			; pop hl
	jr	GLETInsert
GLETChainPushFunction:
	ld	a, e
	ld	b, OutputInDE
	call	GetFunction
	ld	a, 0E1h
	call	InsertA			; pop hl
_:	jr	GLETInsert
GLETChainAnsXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, GLETChainAnsNumber
	dec	a
	jr	z, GLETChainAnsVariable
	dec	a
	jr	z, GLETChainAnsChainPush
	dec	a
	jr	z, GLETChainAnsChainAns
	dec	a
	jp	z, GLETChainAnsFunction
	jp	GLETError
GLETChainAnsNumber:
	call	MaybeChangeDEToHL
	ld	hl, tempToken
	ld	a, (hl)
	cp	tGT
	jr	nz, +_
	inc	de
	ld	a, tGE
	ld	(hl), a
_:	ld	a, 011h
	ex	de, hl
	call	InsertAHL		; ld de, *
	jr	--_
GLETChainAnsVariable:
	call	MaybeChangeDEToHL
	call	InsertIXE		; ld de, (ix+*)
	jr	--_
GLETChainAnsChainPush:
	jp	UnknownError
GLETChainAnsChainAns:
	jp	UnknownError
GLETChainAnsFunction:
	ld	a, (ExprOutput)
	add	a, a
	add	a, a
	add	a, a
	add	a, a
	add	a, 0D5h
	call	InsertA			; push hl/de
	ld	a, e
	ld	b, OutputInDE
	call	GetFunction
	ld	a, 0E1h
	call	InsertA			; pop hl
_:	jr	--_
GLETFunctionXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, GLETFunctionNumber
	dec	a
	jr	z, GLETFunctionVariable
	dec	a
	jr	z, GLETFunctionChainPush
	dec	a
	jr	z, GLETFunctionChainAns
	dec	a
	jr	z, GLETFunctionFunction
	jp	GLETError
GLETFunctionNumber:
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jp	GLETChainAnsNumber
GLETFunctionVariable:
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jp	GLETChainAnsVariable
GLETFunctionChainPush:
	jp	UnknownError
GLETFunctionChainAns:
	ld	a, (ExprOutput)
	add	a, a
	add	a, a
	add	a, a
	add	a, a
	add	a, 0D5h
	call	InsertA			; push hl/de
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	ld	a, 0D1h
	call	InsertA			; pop de
	jr	-_
GLETFunctionFunction:
	ld	a, e
	ld	b, OutputInDE
	call	GetFunction
	ld	a, c
	ld	b, OutputInHL
	set	need_push, (iy+fExpression1)
	call	GetFunction
	jr	-_
	
NEQNumberXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, NEQNumberNumber
	dec	a
	jr	z, NEQNumberVariable
	dec	a
	jr	z, NEQNumberChainPush
	dec	a
	jr	z, NEQNumberChainAns
	dec	a
	jr	z, NEQNumberFunction
	jp	NEQError
NEQNumberNumber:
	set	output_is_number, (iy+fExpression1)
	ex	de, hl
	ld	a, (tempToken)
	cp	tEq
	jr	z, +_
	or	a, a
	sbc	hl, bc
	ld	hl, 0
	jr	z, $+3
	inc	hl
	jr	++_
_:	sbc	hl, bc
	ld	hl, 0
	jr	nz, $+3
	inc	hl
_:	ld	(ix-7), hl
	ret
NEQNumberVariable:
	push	bc
	pop	hl
	ex	de, hl
	push	hl
	pop	bc
	jr	NEQVariableNumber
NEQNumberChainPush:
	jp	UnknownError
NEQNumberChainAns:
	push	bc
	pop	de
	jp	NEQChainAnsNumber
NEQNumberFunction:
	push	bc
	pop	hl
	ex	de, hl
	push	hl
	pop	bc
	jp	NEQFunctionNumber
NEQVariableXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, NEQVariableNumber
	dec	a
	jr	z, NEQVariableVariable
	dec	a
	jr	z, NEQVariableChainPush
	dec	a
	jr	z, NEQVariableChainAns
	dec	a
	jp	z, NEQVariableFunction
	jp	NEQError
NEQVariableNumber:
	call	InsertHIXC		; ld hl, (ix+*)
	jp	NEQChainAnsNumber
NEQVariableVariable:
	ld	a, c
	cp	e
	jr	nz, ++_
	set	output_is_number, (iy+fExpression1)
	ld	a, (tempToken)
	sub	tNE
	jr	z, +_
	ld	a, 1
_:	ld	de, 0
	ld	e, a
	ld	(hl), typeNumber
	inc	hl
	ld	(hl), de
	ret
_:	call	InsertHIXC		; ld hl, (ix+*)
	jp	NEQChainAnsVariable
NEQVariableChainPush:
	jp	UnknownError
NEQVariableChainAns:
	ld	e, c
	jp	NEQChainAnsVariable
NEQVariableFunction:
	ld	a, e
	ld	b, OutputInDE
	call	GetFunction
	call	InsertHIXC		; ld hl, (ix+*)
NEQInsert:
	ld	hl, 052EDB7h
	call	InsertHL		; or a \ sbc hl, de
	ld	de, 0
	ld	hl, 0230120h
	ld	a, (tempToken)
	cp	tEq
	jr	z, +_
	ld	l, 028h
_:	ld	a, 021h
	jp	InsertADEHL		; ld hl, 0 \ jr [n]z, $+3 \ inc hl
NEQChainPushXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, NEQChainPushNumber
	dec	a
	jr	z, NEQChainPushVariable
	dec	a
	jr	z, NEQChainPushChainPush
	dec	a
	jr	z, NEQChainPushChainAns
	dec	a
	jp	z, NEQChainPushFunction
	jp	NEQError
NEQChainPushNumber:
	ld	a, 0E1h
	call	InsertA			; pop hl
	jr	NEQChainAnsNumber
NEQChainPushVariable:
	ld	a, 0E1h
	call	InsertA			; pop hl
	jr	NEQChainAnsVariable
NEQChainPushChainPush:
	jp	UnknownError
NEQChainPushChainAns:
	ld	a, (ExprOutput)
	or	a, a
	ld	a, 0D1h
	jr	nz, +_
	ld	a, 0E1h
_:	call	InsertA			; pop hl/de
	jr	NEQInsert
NEQChainPushFunction:
	ld	a, e
	ld	b, OutputInHL
	call	GetFunction
	ld	a, 0D1h
	call	InsertA			; pop de
_:	jr	NEQInsert
NEQChainAnsXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, NEQChainAnsNumber
	dec	a
	jr	z, NEQChainAnsVariable
	dec	a
	jr	z, NEQChainAnsChainPush
	dec	a
	jr	z, NEQChainAnsChainAns
	dec	a
	jp	z, NEQChainAnsFunction
	jp	NEQError
NEQChainAnsNumber:
	bit	op_is_last_one, (iy+fExpression1)
	jr	z, +_
	ld	a, (tempToken)
	cp	tNE
	jr	nz, +_
	res	ans_set_z_flag, (iy+fExpression1)
_:	ld	a, (ExprOutput)
	or	a, a
	ld	a, 011h
	jr	nz, $+4
	ld	a, 021h
	ex	de, hl
	call	InsertAHL		; ld hl/de, *
	jr	--_
NEQChainAnsVariable:
	ld	h, 017h
	ld	d, 3
	mlt	de
	ld	a, e
	ld	l, 0DDh
	call	_SetHLUToA
	ld	a, (ExprOutput)
	or	a, a
	jr	nz, $+4
	ld	h, 027h
	call	InsertHL		; ld hl/de, (ix+*)
	jr	--_
NEQChainAnsChainPush:
	jp	UnknownError
NEQChainAnsChainAns:
	jp	UnknownError
NEQChainAnsFunction:
	ld	a, (ExprOutput)
	add	a, a
	add	a, a
	add	a, a
	add	a, a
	add	a, 0D5h
	call	InsertA			; push hl/de
	ld	a, e
	ld	b, OutputInHL
	call	GetFunction
	ld	a, 0D1h
	call	InsertA			; pop de
_:	jp	NEQInsert
NEQFunctionXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, NEQFunctionNumber
	dec	a
	jr	z, NEQFunctionVariable
	dec	a
	jr	z, NEQFunctionChainPush
	dec	a
	jr	z, NEQFunctionChainAns
	dec	a
	jr	z, NEQFunctionFunction
	jp	NEQError
NEQFunctionNumber:
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jp	NEQChainAnsNumber
NEQFunctionVariable:
	ld	a, c
	ld	c, e
	ld	e, a
	jp	NEQVariableFunction
NEQFunctionChainPush:
	jp	UnknownError
NEQFunctionChainAns:
	ld	e, c
	jr	NEQChainAnsFunction
NEQFunctionFunction:
	ld	a, e
	ld	b, OutputInDE
	call	GetFunction
	ld	a, c
	ld	b, OutputInHL
	set	need_push, (iy+fExpression1)
	call	GetFunction
	jr	-_

XORANDNumberXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, XORANDNumberNumber
	dec	a
	jr	z, XORANDNumberVariable
	dec	a
	jr	z, XORANDNumberChainPush
	dec	a
	jr	z, XORANDNumberChainAns
	dec	a
	jr	z, XORANDNumberFunction
	jp	XORANDError
XORANDNumberNumber:
	set	output_is_number, (iy+fExpression1)
	or	a, a
	sbc	hl, hl
	ld	a, (tempToken)
	cp	tXor
	jr	z, ++_
	cp	tOr
	jr	z, +_
	call	_ChkBCIs0
	jr	z, +++_
	call	_ChkDEIs0
	jr	z, +++_
	inc	hl
	jr	+++_
_:	call	_ChkBCIs0
	ld	b, a
	call	_ChkDEIs0
	or	a, b
	jr	z, ++_
	inc	hl
	jr	++_
_:	call	_ChkDEIs0
	ld	de, 0
	jr	z, $+3
	inc	de
	call	_ChkBCIs0
	ld	a, 0
	jr	z, $+3
	inc	a
	xor	e
	ld	e, a
	ex	de, hl
_:	ld	(ix-7), hl
	ret
XORANDNumberVariable:
	push	bc
	pop	hl
	ex	de, hl
	push	hl
	pop	bc
	jr	XORANDVariableNumber
XORANDNumberChainPush:
	jp	UnknownError
XORANDNumberChainAns:
	push	bc
	pop	de
	jp	XORANDChainAnsNumber
XORANDNumberFunction:
	push	bc
	pop	hl
	ex	de, hl
	push	hl
	pop	bc
	jp	XORANDFunctionNumber
XORANDVariableXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, XORANDVariableNumber
	dec	a
	jr	z, XORANDVariableVariable
	dec	a
	jr	z, XORANDVariableChainPush
	dec	a
	jr	z, XORANDVariableChainAns
	dec	a
	jp	z, XORANDVariableFunction
	jp	XORANDError
XORANDVariableNumber:
	call	InsertHIXC		; ld hl, (ix+*)
	jp	XORANDChainAnsNumber
XORANDVariableVariable:
	ld	a, c
	cp	e
	jr	nz, ++_
	set	output_is_number, (iy+fExpression1)
	ld	a, (tempToken)
	sub	tNE
	jr	z, +_
	ld	a, 1
_:	ld	de, 0
	ld	e, a
	ld	(hl), typeNumber
	inc	hl
	ld	(hl), de
	ret
_:	call	InsertHIXC		; ld hl, (ix+*)
	jp	XORANDChainAnsVariable
XORANDVariableChainPush:
	jp	UnknownError
XORANDVariableChainAns:
	ld	e, c
	jp	XORANDChainAnsVariable
XORANDVariableFunction:
	ld	a, e
	ld	b, OutputInDE
	call	GetFunction
	call	InsertHIXC		; ld hl, (ix+*)
XORANDInsert:
	ld	de, (programPtr)
	ld	hl, XORANDData
	ld	b, 0A2h
	ld	a, (tempToken)
	cp	tXor
	jr	nz, +_
	ld	b, 0AAh
_:	cp	tOr
	jr	nz, +_
	ld	b, 0B2h
_:	ld	a, b
	ld	(XORANDSMC), a
	ld	bc, 16
	ldir
	ld	(programPtr), de
	ret
XORANDChainPushXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, XORANDChainPushNumber
	dec	a
	jr	z, XORANDChainPushVariable
	dec	a
	jr	z, XORANDChainPushChainPush
	dec	a
	jr	z, XORANDChainPushChainAns
	dec	a
	jp	z, XORANDChainPushFunction
	jp	XORANDError
XORANDChainPushNumber:
	ld	a, 0E1h
	call	InsertA			; pop hl
	jr	XORANDChainAnsNumber
XORANDChainPushVariable:
	ld	a, 0E1h
	call	InsertA			; pop hl
	jr	XORANDChainAnsVariable
XORANDChainPushChainPush:
	jp	UnknownError
XORANDChainPushChainAns:
	ld	a, 0D1h
	call	InsertA			; pop de
	jr	XORANDInsert
XORANDChainPushFunction:
	ld	a, e
	ld	b, OutputInHL
	call	GetFunction
	ld	a, 0D1h
	call	InsertA			; pop de
_:	jr	XORANDInsert
XORANDChainAnsXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, XORANDChainAnsNumber
	dec	a
	jr	z, XORANDChainAnsVariable
	dec	a
	jr	z, XORANDChainAnsChainPush
	dec	a
	jr	z, XORANDChainAnsChainAns
	dec	a
	jp	z, XORANDChainAnsFunction
	jp	XORANDError
XORANDChainAnsNumber:
	ld	a, (ExprOutput)
	or	a, a
	ld	a, 011h
	jr	nz, +_
	ld	a, 021h
_:	ex	de, hl
	call	InsertAHL		; ld hl/de, *
	jr	--_
XORANDChainAnsVariable:
	ld	h, 017h
	ld	d, 3
	mlt	de
	ld	a, e
	ld	l, 0DDh
	call	_SetHLUToA
	ld	a, (ExprOutput)
	or	a, a
	jr	nz, $+4
	ld	h, 027h
	call	InsertHL		; ld hl/de, (ix+*)
	jr	--_
XORANDChainAnsChainPush:
	jp	UnknownError
XORANDChainAnsChainAns:
	jp	UnknownError
XORANDChainAnsFunction:
	ld	a, (ExprOutput)
	add	a, a
	add	a, a
	add	a, a
	add	a, a
	add	a, 0D5h
	call	InsertA			; push hl/de
	ld	a, e
	ld	b, OutputInHL
	call	GetFunction
	ld	a, 0D1h
	call	InsertA			; pop de
_:	jr	---_
XORANDFunctionXXX:
	ld	a, (ix-4)
	or	a, a
	jr	z, XORANDFunctionNumber
	dec	a
	jr	z, XORANDFunctionVariable
	dec	a
	jr	z, XORANDFunctionChainPush
	dec	a
	jr	z, XORANDFunctionChainAns
	dec	a
	jr	z, XORANDFunctionFunction
	jp	XORANDError
XORANDFunctionNumber:
	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	jp	XORANDChainAnsNumber
XORANDFunctionVariable:
	ld	a, c
	ld	c, e
	ld	e, a
	jp	XORANDVariableFunction
XORANDFunctionChainPush:
	jp	UnknownError
XORANDFunctionChainAns:
	ld	e, c
	jr	XORANDChainAnsFunction
XORANDFunctionFunction:
	ld	a, e
	ld	b, OutputInDE
	call	GetFunction
	ld	a, c
	ld	b, OutputInHL
	set	need_push, (iy+fExpression1)
	call	GetFunction
	jr	-_
	
StoNumberXXX:
	ld	a, (ix-4)
	cp	typeChainAns
	jr	z, +_
	cp	typeVariable
	jp	z, ++_
	jp	UnknownError
_:	call	StoTestStoToListElement
	jp	nz, UnknownError
	ld	a, 011h
	push	bc
	pop	hl
	call	InsertAHL		; ld de, *
	ld	a, 0EDh
	call	InsertA			; ld (hl), de (1)
	ld	a, 01Fh
	call	InsertA			; ld (hl), de (2)
	ld	a, OutputIsInDE
	ld	(ExprOutput2), a
	ret
_:	ld	hl, 0002FDDh
	ld	d, 3
	mlt	de
	ld	a, e
	call	_SetHLUToA
	push	bc
	pop	de
	call	_ChkBCIs0
	ld	a, 021h
	jr	nz, +_
	ld	de, 062EDB7h
	jp	InsertDEHL		; or a \ sbc hl, hl \ ld (ix+*), hl
_:	jp	InsertADEHL		; ld hl, * \ ld (ix+*), hl
StoVariableXXX:
	ld	a, (ix-4)
	cp	typeChainAns
	jr	z, +_
	cp	typeVariable
	jp	z, ++_
	jp	UnknownError
_:	call	StoTestStoToListElement
	jp	nz, UnknownError
	ld	e, c
	call	InsertIXE		; ld de, (ix+*)
	ld	a, 0EDh
	call	InsertA			; ld (hl), de (1)
	ld	a, 01Fh
	call	InsertA			; ld (hl), de (2)
	ld	a, OutputIsInDE
	ld	(ExprOutput2), a
	ret
_:	call	InsertHIXC
	ld	hl, 0002FDDh
	ld	d, 3
	mlt	de
	ld	a, e
	call	_SetHLUToA
	jp	InsertHL		; ld ld (ix+*), hl
StoChainPushXXX:
	ld	a, (ix-4)
	cp	typeChainAns
_:	jp	nz, UnknownError
	call	StoTestStoToListElement
	jr	nz, -_
	ld	hl, 01FEDD1h
	call	InsertHL		; pop de \ ld (hl), de
	ld	a, OutputIsInDE
	ld	(ExprOutput2), a
	ret
StoChainAnsXXX:
	ld	a, (ix-4)
	cp	typeVariable
	jp	nz, StoError
_:	ld	hl, 0002FDDh
	ld	d, 3
	mlt	de
	ld	a, e
	call	_SetHLUToA
	ld	a, (ExprOutput)
	or	a, a
	jr	nz, $+4
	ld	h, 01Fh
	ld	(ExprOutput2), a
	jp	InsertHL		; ld (ix+*), hl/de
StoFunctionXXX:
	ld	a, (ix-4)
	cp	typeChainAns
	jr	z, +_
	cp	typeVariable
	jp	z, ++_
	jp	UnknownError
_:	call	StoTestStoToListElement
	jp	nz, UnknownError
	ld	a, c
	ld	b, OutputInDE
	set	need_push, (iy+fExpression1)
	call	GetFunction
	ld	a, 0EDh
	call	InsertA			; ld (hl), de (1)
	ld	a, 01Fh
	call	InsertA			; ld (hl), de (2)
	ld	a, OutputIsInDE
	ld	(ExprOutput2), a
	ret
_:	ld	a, c
	ld	b, OutputInHL
	call	GetFunction
	ld	hl, 0002FDDh
	ld	d, 3
	mlt	de
	ld	a, e
	call	_SetHLUToA
	jp	InsertHL		; ld (ix+*), hl
StoListXXX:
	ld	a, (ix-4)
	cp	typeOSList
	jp	nz, StoError
	ld	a, 021h
	call	InsertA			; ld hl, *
	call	InsertProgramPtrToDataOffset
	ld	hl, (programDataDataPtr)
	call	InsertHL		; ld hl, RANDOM
	ld	a, 011h
	push	de
	pop	hl
	call	InsertAHL		; ld de, OSList
	ld	a, 01h
	call	InsertA			; ld bc, *
	push	bc
	pop	hl
	ld	(StartData), hl
	ld	hl, (hl)
	push	hl
	pop	bc
	call	InsertHL		; ld bc, LENGTH
	ld	de, (programDataDataPtr)
StartData = $+1
	ld	hl, 0
	inc	hl
	inc	hl
	inc	hl
	ldir
	ld	(programDataDataPtr), de
	ld	a, 0EDh
	call	InsertA			; ldir (1)
	ld	a, 0B0h
	jp	InsertA			; ldir (2)
	
StoTestStoToListElement:
	ld	hl, (programPtr)
	dec	hl
	ld	a, (hl)
	cp	027h
	jr	nz, StoLastOpcodeNotHLHL
	dec	hl
	ld	a, (hl)
	cp	0EDh
	jr	nz, StoLastOpcodeNotHLHL
	ld	(programPtr), hl
	ret
StoLastOpcodeNotHLHL:
	ld	hl, (programPtr)
	dec	hl
	dec	hl
	dec	hl
	dec	hl
	ld	a, (hl)
	cp	02Ah
	ret	nz
	ld	(hl), 021h
	ret
	nop
	nop
	nop