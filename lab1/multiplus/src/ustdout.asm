; eax: str to be printed
SECTION .text
global ustdout
extern ustrl
extern ukernel

ustdout:
	; store eax ebx ecx edx
	push eax
	push ebx
	push ecx
	push edx

	; str
	mov ecx, eax
	call ustrl
	; return value in eax
	mov edx, eax
	; sys_write
	mov eax, 4
	; stdout
	mov ebx, 1
	
	call ukernel
	
.end:
	pop edx
	pop ecx
	pop ebx
	pop eax
	ret

