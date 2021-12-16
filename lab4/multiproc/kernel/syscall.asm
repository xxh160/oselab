%include "sconst.inc"

_NR_get_ticks equ 0 ; 要跟 global.c 中 sys_call_table 的定义相对应！
_NR_milli_skip equ 1
_NR_print equ 2
_NR_p equ 3
_NR_v equ 4
INT_VECTOR_SYS_CALL equ 0x90

; 导出符号
global get_ticks
global milli_skip
global print
global p
global v

bits 32
[section .text]

get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	ret

milli_skip:
	push ebp
	mov ebp, esp
	mov eax, _NR_milli_skip
	push ebx
	; hwd: milli_sec
	mov ebx, [ebp + 8]
	int INT_VECTOR_SYS_CALL
	pop ebx
	pop ebp
	ret

print:
	push ebp
	mov ebp, esp
	mov eax, _NR_print
	push ebx
	mov ebx, [ebp + 8]
	int INT_VECTOR_SYS_CALL
	pop ebx
	pop ebp
	ret

p:
	push ebp
	mov ebp, esp
	mov eax, _NR_p
	push ebx
	mov ebx, [ebp + 8]
	int INT_VECTOR_SYS_CALL
	pop ebx
	pop ebp
	ret

v:
	push ebp
	mov ebp, esp
	mov eax, _NR_v
	push ebx
	mov ebx, [ebp + 8]
	int INT_VECTOR_SYS_CALL
	pop ebx
	pop ebp
	ret

