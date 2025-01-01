
	bits	64

	; kernel32.dll
	extern	ExitProcess
	extern	GetModuleHandleW
	extern	GetCommandLineW
	extern	GetStartupInfoW

	extern	MyWinMain

section CODE code
	global	start
start:
	sub	rsp, 0x28

	xor	rcx, rcx
	call	GetModuleHandleW
	mov	[rel hInstance], rax

; based on https://github.com/mingw-w64/mingw-w64/blob/master/mingw-w64-crt/crt/crtexewin.c

	call	GetCommandLineW
.check_space:
	test	word [rax], 0xFFFF
	je	.end_of_command
	inc	rax
	inc	rax
	cmp	word [rax-2], ' '
	jle	.end_of_command
	cmp	word [rax-2], '"'
	je	.check_quote
	jmp	.check_space

.check_quote:
	test	word [rax], 0xFFFF
	je	.end_of_command
	inc	rax
	inc	rax
	cmp	word [rax-2], '"'
	je	.check_space
	jmp	.check_quote

.end_of_command:
	mov	[rel lpszCmdLine], rax

; based on https://github.com/mingw-w64/mingw-w64/blob/master/mingw-w64-crt/crt/crtexewin.c
	sub	rsp, 112 ; STARTUPINFOA (actually 104, but must be 16-byte aligned)
	mov	rcx, rsp
	call	GetStartupInfoW
	test	dword [rsp + 60], 1 ; dwFlags & STARTF_USESHOWWINDOW
	jz	.use_showdefault
	mov	eax, [rsp + 64] ; wShowWindow
	mov	[rel nCmdShow], eax
.use_showdefault:
	add	rsp, 112

	mov	r9d, [rel nCmdShow]
	mov	r8, [rel lpszCmdLine]
	xor	rdx, rdx	; hPrevInstance
	mov	rcx, [rel hInstance]
	call	MyWinMain

	mov	rcx, rax
	call	ExitProcess

; libc is looking for this, so provide it
	extern	WinMain
WinMain:
	ret

section DATA data
hInstance:
	dq	0

lpszCmdLine:
	dq	0

nCmdShow:
	dd	10	; SW_SHOWDEFAULT

