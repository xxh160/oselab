%ifndef __USUBZERO_ASM
%define __USUBZERO_ASM

%include "constant.asm"

; 在 usplit 之前调用
; eax: addr
SECTION .text
global usubzero

usubzero:
	push ebx
	mov ebx, eax
	
.loop:
	cmp byte[ebx], ZERO
	jz .end
	cmp byte[ebx], SPACE
	jz .back
	jmp .sub	

.back:
	inc ebx
	jmp .loop
	
.sub:
	sub byte[ebx], CZERO 
	jmp .back

.end:
	pop ebx
	ret

%endif
