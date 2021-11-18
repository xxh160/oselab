; eax addr
; ebx length
SECTION .text
global ustdin
extern ukernel

ustdin:
	push eax
	push ecx
	push edx
	push ebx
	
	mov ecx, eax
	mov edx, ebx
	
	mov eax, 3
	mov ebx, 1
	
	call ukernel
	
	pop ebx
	pop edx
	pop ecx
	pop eax
	
	ret

