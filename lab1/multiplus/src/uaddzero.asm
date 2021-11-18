%include "constant.asm"

SECTION .text
global uaddzero
; eax: begin addr
; ebx: end addr
uaddzero:
	push ecx
	mov ecx, eax
	
.loop:
	cmp ecx, ebx
	je .end
	cmp byte[ecx], NEGATIVE
	je .continue
	add byte[ecx], CZERO

.continue:
	inc ecx
	jmp .loop

.end:
	pop ecx
	ret

