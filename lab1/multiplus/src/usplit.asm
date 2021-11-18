; eax addr
; return: eax: the second addr
%include "constant.asm"

SECTION .text
global usplit

usplit:
	push ebx
	mov ebx, eax
	
.next:
	cmp byte[ebx], SPACE
	je .spacel
	inc ebx
	jmp .next

.spacel:
	cmp byte[ebx], SPACE
	jne .end
	mov byte[ebx], ZERO
	inc ebx
	jmp .spacel	

.end:
	mov eax, ebx
	pop ebx
	ret

