%include "constant.asm"
%include "macrofunc.asm"
; extern
extern ustdout
extern ustdin
extern ustrip
extern usubzero
extern usplit
extern ustrl
extern uexit 
extern uaddzero
extern unegative

; 小端系统 wdnmd
; 打印需要加上 '0'
; 所有函数都是无寄存器副作用的 除非需要用 eax 保存 return value
; ZERO 保证 ustrl 可以正常运行
SECTION .data                    
input_msg: db "Plz input num $A and num $B:", ENDL, ZERO
add_msg: db "$A + $B:", ENDL, ZERO
multi_msg: db "$A * $B:", ENDL, ZERO
endl: db ENDL, ZERO
space: db SPACE, ZERO

SECTION .bss
; input length
ab_n: resb 2 * NUM_SIZE + SPACE_SIZE + 2 * SYMBOL_SIZE 
; a start address
a_p: resb	PTR_SIZE 
; b start address
b_p: resb PTR_SIZE
; a length
a_l: resb LEN_SIZE
; b length
b_l: resb LEN_SIZE
; boolean: is a negative
a_neg: resb SYMBOL_SIZE
; boolean: ...
b_neg: resb SYMBOL_SIZE
; add result
res_add: resb NUM_SIZE + SYMBOL_SIZE
; mul result
res_multi: resb NUM_SIZE + SYMBOL_SIZE
; split
zero: resb 1
; ignore unused space
; add result start addr
add_addr: resb PTR_SIZE
; mul result start addr
multi_addr: resb PTR_SIZE

SECTION .text 
global _start 

_start:
	mov byte[zero], ZERO
	; print input msg
	mov eax, input_msg
	call ustdout
	
	; get num_str
	mov eax, ab_n
	mov ebx, 2 * NUM_SIZE + SPACE_SIZE + 2 * SYMBOL_SIZE
	call ustdin
	mov eax, ab_n
	; remove \n
	call ustrip
	
	; split nums
	; $A addr
	mov dword[a_p], ab_n
	mov eax, ab_n
	; convert space to \0
	call usplit
	; $B addr
	mov dword[b_p], eax
	
	; judge $A symbol
	mov eax, dword[a_p]
	call unegative
	mov dword[a_neg], eax
	cmp eax, TRUE
	jne .calalen
	inc dword[a_p]
	; calcualte $A len
.calalen:
	mov eax, dword[a_p]
	call ustrl
	mov dword[a_l], eax

	; judge $B symbol
	mov eax, dword[b_p]
	call unegative
	mov dword[b_neg], eax
	cmp eax, TRUE	
	jne .calblen	
	inc dword[b_p]
	; calculate $B len
.calblen:
	mov eax, dword[b_p]
	call ustrl
	mov dword[b_l], eax 
	
	; sub '0'
	mov eax, dword[a_p]
	call usubzero
	mov eax, dword[b_p]
	call usubzero

	; mov eax, [a_p]
	; mov ebx, [a_l]
	; mov ecx, [b_p]
	; mov edx, [b_l]
	; mov esi, res_add
	; call uadd
	; mov dword[add_addr], eax

	condadd a_p, a_l, a_neg, b_p, b_l, b_neg, res_add, add_addr
	
	; print add_msg
	mov eax, add_msg
	call ustdout
	; begin addr
	mov eax, dword[add_addr]
	; end addr
	mov ebx, res_add
	add ebx, NUM_SIZE
	add ebx, SYMBOL_SIZE
	; add '0'
	call uaddzero
	call ustdout
	; print endl
	mov eax, endl
	call ustdout

	condmulti a_p, a_l, a_neg, b_p, b_l, b_neg, res_multi, multi_addr
	
	; print multi_msg
	mov eax, multi_msg
	call ustdout
	; begin addr
	mov eax, dword[multi_addr]
	; end addr
	mov ebx, res_multi
	add ebx, NUM_SIZE
	add ebx, SYMBOL_SIZE
	; add '0'
	call uaddzero
	call ustdout
	; print endl
	mov eax, endl
	call ustdout

	call uexit

