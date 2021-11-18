%include "constant.asm"

SECTION .text
global ustrl
; eax: str addr
; return: in eax, strl
ustrl:
	; store ebx 
	push ebx
	mov ebx, eax

.next:
	cmp byte[ebx], ZERO
	jz .end
	inc ebx
	jmp .next	

.end:
	; calculate the bytes
	sub ebx, eax
	mov eax, ebx
	pop ebx
	ret
