
	bits	32

	; kernel32.dll
	extern	_ExitProcess@4
%ifndef	UNICODE
	extern	_GetModuleHandleA@4
	extern	_GetCommandLineA@0
	extern	_GetStartupInfoA@4
%else
	extern	_GetModuleHandleW@4
	extern	_GetCommandLineW@0
	extern	_GetStartupInfoW@4
%endif
	extern	_MyWinMain@16

%ifndef	GNU
section CODE class=CODE USE32
%else
section CODE code
%endif
	global	start
start:

	push	dword 0
%ifndef	UNICODE
	call	_GetModuleHandleA@4
%else
	call	_GetModuleHandleW@4
%endif
	mov	[hInstance], eax

; based on https://github.com/mingw-w64/mingw-w64/blob/master/mingw-w64-crt/crt/crtexewin.c
%ifndef	UNICODE
	call	_GetCommandLineA@0
.check_space:
	test	byte [eax], 0xFF
	je	.end_of_command
	inc	eax
	cmp	byte [eax-1], ' '
	jle	.end_of_command
	cmp	byte [eax-1], '"'
	je	.check_quote
	jmp	.check_space

.check_quote:
	test	byte [eax], 0xFF
	je	.end_of_command
	inc	eax
	cmp	byte [eax-1], '"'
	je	.check_space
	jmp	.check_quote
%else
	call	_GetCommandLineW@0
.check_space:
	test	word [eax], 0xFFFF
	je	.end_of_command
	inc	eax
	inc	eax
	cmp	word [eax-2], ' '
	jle	.end_of_command
	cmp	word [eax-2], '"'
	je	.check_quote
	jmp	.check_space

.check_quote:
	test	word [eax], 0xFFFF
	je	.end_of_command
	inc	eax
	inc	eax
	cmp	word [eax-2], '"'
	je	.check_space
	jmp	.check_quote
%endif

.end_of_command:
	mov	[lpszCmdLine], eax

; based on https://github.com/mingw-w64/mingw-w64/blob/master/mingw-w64-crt/crt/crtexewin.c
	sub	esp, 68 ; STARTUPINFOA
	push	esp
%ifndef	UNICODE
	call	_GetStartupInfoA@4
%else
	call	_GetStartupInfoW@4
%endif
	test	dword [esp + 44], 1 ; dwFlags & STARTF_USESHOWWINDOW
	jz	.use_showdefault
	mov	eax, [esp + 48] ; wShowWindow
	mov	[nCmdShow], eax
.use_showdefault:
	add	esp, 68

	push	dword [nCmdShow]
	push	dword [lpszCmdLine]
	push	dword 0	; hPrevInstance
	push	dword [hInstance]
	call	_MyWinMain@16

	push	eax
	call	_ExitProcess@4

%ifdef	GNU
; libc is looking for this, so provide it
	extern	_WinMain@16
_WinMain@16:
	ret
%endif

%ifndef	GNU
section BEGDATA class=BEGDATA USE32
%else
section DATA data
%endif
hInstance:
	dd	0

lpszCmdLine:
	dd	0

nCmdShow:
	dd	10	; SW_SHOWDEFAULT

%ifndef	GNU
group DGROUP BEGDATA
%endif

