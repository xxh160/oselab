%include "constant.asm"

SECTION .data
; return state, state == 2 ==> .end
state: db 2
carry: dd 0
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
global uadd

; esi is addr register
uadd:
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
	je .add
	dec byte[state]
	; $B cur pointer
	mov esi, dword[bbase]
	add esi, dword[bi]
	mov ah, byte[esi]

.add:
	cmp byte[state], 2 
	je .end
	; a_cur + b_cur + carry
	add al, ah
	add al, byte[carry]
	cmp al, 10
	jnb .hascarry
	mov dword[carry], 0
	jmp .write

.hascarry:
	mov dword[carry], 1
	sub al, 10

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
	cmp dword[carry], 1
	je .addone
	inc dword[ri]
	jmp .last

.addone:
	mov ebx, dword[ri]
	mov byte[ebx], 1	
	
.last:
	mov eax, dword[ri]
	pop esi
	pop edx
	pop ecx
	pop ebx
	ret	

