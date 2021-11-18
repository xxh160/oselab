global xprint

section .text
; eax ecx edx -> caller saved
xprint:
	; save old %ebp
	push ebp
	; modify %ebp
	mov ebp, esp
	; ebx esi edi -> callee saved
	push ebx

	mov ecx, dword[ebp + 0x10]
	mov edx, dword[ebp + 0x14] 
	mov eax, 4
	mov ebx, 1

	int 0x80
	
.output:

	mov ecx, dword[ebp + 0x08]
	mov edx, dword[ebp + 0x0c]
	mov eax, 4
	mov ebx, 1	
	
	int 0x80

	pop ebx
	; restore old %ebp
	pop ebp
	; return
	ret

