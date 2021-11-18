%include "constant.asm"

SECTION .data
state: db 2
borrow: dd 0
; $A base addr
abase: dd 0
; $B base addr
bbase: dd 0
; $A cur pointer without base addr
ai: dd 0
; $B ...
bi: dd 0
; result addr
ri: dd NUM_SIZE + SYMBOL_SIZE - 1
; eax: a_p, ebx: a_l, ecx: b_p, edx: b_l
; esi: res_add
; local vars are all in .data
; return: eax: valid start addr
SECTION .text
global usub

; a - b, and abs(a) >= abs(b)
; esi is addr register
usub:
	; init
	add dword[abase], eax
	add dword[bbase], ecx
	add dword[ai], ebx
	sub dword[ai], 1
	add dword[bi], edx
	sub dword[bi], 1
	add dword[ri], esi
	; store
	push ebx
	push ecx
	push edx
	push esi

.loop:
	mov al, 0
	mov ah, 0
	cmp dword[ai], -1
	je .getb
	dec byte[state]

.geta:
	; $A cur pointer
	mov esi, dword[abase]
	add esi, dword[ai]
	mov al, byte[esi]

.getb:
	cmp dword[bi], -1
	je .sub
	dec byte[state]
	; $B cur pointer
	mov esi, dword[bbase]
	add esi, dword[bi]
	mov ah, byte[esi]

.sub:
	cmp byte[state], 2 
	je .end
	; a_cur - b_cur - borrow
	sub al, ah
	sub al, byte[borrow]
	cmp al, 10
	; al < 0
	; unsigned
	jnb .hasborrow
	mov dword[borrow], 0
	jmp .write

.hasborrow:
	mov dword[borrow], 1
	add al, 10

.write:
	mov byte[state], 2
	mov ebx, dword[ri]
	mov byte[ebx], al
	cmp dword[ai], -1
	je .subri	

.subai:
	dec dword[ai]

.subri:
	dec dword[ri]
	cmp dword[bi], -1
	je .loop

.subbi:
	dec dword[bi]
	jmp .loop

.end:
	inc dword[ri]
	mov eax, dword[ri]

.incaddr:
	cmp byte[eax], ZERO
	jne .last
	inc eax	
	jmp .incaddr

.last:
	pop esi
	pop edx
	pop ecx
	pop ebx
	ret	

