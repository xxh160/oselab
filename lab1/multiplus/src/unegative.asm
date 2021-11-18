%include "constant.asm"

SECTION .text
global unegative

; eax: addr
; judge if the num is negative
; return: eax: 1 or 0
unegative:
	push ebx
	mov ebx, eax
	cmp byte[ebx], NEGATIVE 			
	je .solve
	mov eax, 0
	jmp .end

.solve:
	mov byte[ebx], ZERO
	mov eax, 1

.end:
	pop ebx
	ret
