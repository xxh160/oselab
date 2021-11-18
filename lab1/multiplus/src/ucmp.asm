SECTION .text
global ucmp

; eax: addr $A
; ebx: len $A
; ecx: addr $B
; edx: len $B
; return: eax: 0 -> $A >= $B, 1 -> $A < $B
ucmp:
	push ebx
	push ecx
	push edx

	cmp ebx, edx
	jb .bb
	cmp edx, ebx
	jb .ab
	; al == bl
	add eax, ebx
	dec eax
	add ecx, edx
	dec ecx

.loop:
	cmp ebx, 0
	je .ab 
	mov dh, byte[ecx]
	cmp byte[eax], dh
	jb .bb
	mov dh, byte[eax]
	cmp byte[ecx], dh
	jb .ab 
	dec eax
	dec ebx
	dec ecx
	jmp .loop

.ab:
	mov eax, 0	
	jmp .end

.bb:
	mov eax, 1
	jmp .end

.end:	
	pop edx
	pop ecx
	pop ebx
	ret
