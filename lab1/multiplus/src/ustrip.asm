%include "constant.asm"
; eax: addr 
SECTION .text
global ustrip

ustrip:
	push ebx
	mov ebx, eax

.next:
	cmp byte[ebx], ENDL
	jz .end
	inc ebx
	jmp .next	

.end:
	mov byte[ebx], ZERO	
	pop ebx
	ret	

