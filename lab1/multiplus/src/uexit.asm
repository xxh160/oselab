SECTION .text
global uexit
extern ukernel

uexit:
	push eax
	push ebx
	
	mov eax, 1
	mov ebx, 0
	
	call ukernel	

	; should never reach here
	pop ebx
	pop eax
	ret

