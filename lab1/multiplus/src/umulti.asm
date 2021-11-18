%include "constant.asm"

SECTION .data
; $A base addr
abase: dd 0
; $B base addr
bbase: dd 0
; $A cur pointer
; usage: dword[ri] - dword[ai] + dword[alen]
ai: dd 0
; $B ...
bi: dd 0
; result addr
alen: dd 0
blen: dd 0
ri: dd NUM_SIZE + SYMBOL_SIZE
; eax: a_p, ebx: a_l, ecx: b_p, edx: b_l
; esi: res_multi
; local vars are all in .data
; return: eax: valid start addr
SECTION .text
global umulti

; ebx is addr register
; ecx is num register: cl, ch
; eax, edx is mul/div register
umulti:
	; init
	add dword[abase], eax
	add dword[bbase], ecx
	add dword[ai], 1
	add dword[bi], 1
	add dword[alen], ebx
	add dword[blen], edx
	add dword[ri], esi
	; store
	push ebx
	push ecx
	push edx
	push esi

	cmp dword[alen], 1
	je .aspecial

.checkb:
	cmp dword[blen], 1
	je .bspecial

.outloop:
	; pick cur_b	
	; cur_b
	mov ch, 0
	mov ebx, dword[blen]
	cmp dword[bi], ebx 
	ja .end
	sub ebx, dword[bi]
	; bbase + blen - bi
	add ebx, dword[bbase]
	mov ch, byte[ebx]

.inloop:
	; pick cur_a
	; cur_a
	mov cl, 0
	mov ebx, dword[alen]
	cmp dword[ai], ebx
	ja .endloop
	sub ebx, dword[ai]
	add ebx, dword[abase]
	mov cl, byte[ebx]
	; al <- ch == cur_b
	mov al, ch
	; ax = al * cl == ch * cl == cur_b * cur_a
	mul cl
	; ah <- ax % 10, al <- ax / 10
	mov dl, 10
	div dl
	mov ebx, dword[ri]
	sub ebx, dword[ai]
	sub ebx, dword[bi]
	; [[ri] - ai - bi] += ax / 10
	add byte[ebx], al
	inc ebx
	; [[ri] - ai - bi + 1] += ax % 10
	add byte[ebx], ah
	; [[ri] - ai - bi + 1] %= 10
	mov al, byte[ebx]
	mov dl, 1
	mul dl
	mov dl, 10
	div dl
	mov byte[ebx], ah
	; if byte[ebx] >= 10
	dec ebx
	add byte[ebx], al
	; change ai
	inc dword[ai]
	jmp .inloop

.endloop:
	; set ai to 1
	mov dword[ai], 1
	; change bi
	inc dword[bi]	
	jmp .outloop

.end:
	mov eax, dword[ri]
	sub eax, dword[alen]
	sub eax, dword[blen]

.incaddr:
	cmp byte[eax], ZERO
	jne .last
	inc eax
	jmp .incaddr
	
.aspecial:
	mov eax, dword[abase] 
	cmp byte[eax], ZERO
	je .zero
	jmp .checkb

.bspecial:
	mov eax, dword[bbase] 
	cmp byte[eax], ZERO
	je .zero
	jmp .outloop

.zero:
	mov eax, dword[ri]	
	dec eax
	mov byte[eax], ZERO

.last:
	pop esi
	pop edx
	pop ecx
	pop ebx
	ret

