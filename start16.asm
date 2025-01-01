
	bits	16

	; krnl386.exe
	extern	InitTask
	extern	WaitEvent
	; user.exe
	extern	InitApp

	extern	MyWinMain

section CODE class=CODE

	global	start
start:
	call	far InitTask
	test	ax, ax
	jnz	noerror

	mov	ax, 0x4C01
	int	0x21

noerror:
	mov	[hInstance], di
	mov	[hPrevInstance], si

	mov	[lpszCmdLine], bx
	mov	[lpszCmdLine + 2], es

	mov	[nCmdShow], dx

	xor	ax, ax
	push	ax
	call	far WaitEvent

	push	word [hInstance]
	call	far InitApp

	push	word [hInstance]
	push	word [hPrevInstance]
	push	word [lpszCmdLine + 2]
	push	word [lpszCmdLine]
	push	word [nCmdShow]

	push	ss
	pop	ds
	push	ss
	pop	es

	call	MyWinMain

	mov	ah, 0x4C
	int	0x21

	global	_small_code_
_small_code_	equ	1

; clib is looking for this, so provide it
	global	main_
main_:
	ret

section BEGDATA class=BEGDATA
	times	16	db	0

hInstance:
	dw	0

hPrevInstance:
	dw	0

lpszCmdLine:
	dd	0

nCmdShow:
	dw	0

group DGROUP BEGDATA
