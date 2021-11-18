%ifndef __MACROFUNC_ASM
%define __MACROFUNC_ASM

%include "constant.asm"
extern uadd
extern usub
extern ucmp
extern umulti

; %5 == True, reverse
%macro allcr 5
	cmp %5, TRUE
	je %%reverse
	mov eax, [%1]
	mov ebx, [%2]
	mov ecx, [%3]
	mov edx, [%4]
	jmp %%end

%%reverse:
	mov eax, [%3]
	mov ebx, [%4]
	mov ecx, [%1]
	mov edx, [%2]

%%end:
	cmp %5, TRUE
%endmacro

; a_p a_l a_neg b_p b_l b_neg res_add add_addr
; return: eax: valid start addr
%macro condadd 8
	; init
	mov esi, %7

	cmp	dword[%3], TRUE
	je %%aneg
	cmp dword[%6], TRUE	
	je %%apbn
	; a >= 0, b >= 0
	mov eax, FALSE
	allcr %1, %2, %4, %5, eax
	call uadd
	jmp %%end

%%apbn:
	; a >= 0, b < 0
	mov eax, FALSE
	allcr %1, %2, %4, %5, eax
	call ucmp
	cmp eax, FALSE
	jne %%apreverse
	; abs(a) >= abs(b)
	mov eax, FALSE
	allcr %1, %2, %4, %5, eax
	call usub
	jmp %%end

%%apreverse:
	; abs(a) < abs(b)
	mov eax, TRUE
	allcr %1, %2, %4, %5, eax
	call usub
	jmp %%addneg

%%aneg:
	cmp dword[%6], TRUE	
	je %%anbn
	; a < 0, b >= 0
	mov eax, FALSE
	allcr %1, %2, %4, %5, eax
	call ucmp
	cmp eax, FALSE
	je %%bpreverse
	; abs(a) < abs(b)
	mov eax, TRUE
	allcr %1, %2, %4, %5, eax
	call usub
	jmp %%end

%%bpreverse:
	; abs(a) >= abs(b)
	mov eax, FALSE
	allcr %1, %2, %4, %5, eax
	call usub
	jmp %%addneg

%%anbn:
	; a < 0, b < 0
	mov eax, FALSE
	allcr %1, %2, %4, %5, eax
	call uadd

%%addneg:
	dec eax
	mov byte[eax], NEGATIVE

%%end:
	mov dword[%8], eax

%endmacro

; a_p a_l a_neg b_p b_l b_neg res_multi multi_addr
; return: eax: valid start addr
%macro condmulti 8
	mov esi, %7
	mov eax, FALSE
	allcr %1, %2, %4, %5, eax
	call umulti
	push eax

	cmp dword[%3], TRUE
	je %%aneg
	; a >= 0
	cmp dword[%6], TRUE
	je %%abn
	; a >= 0, b >= 0
	jmp %%end

%%aneg:
	; a < 0
	cmp dword[%6], TRUE
	; a < 0, b < 0
	je %%end
	
%%abn:
	; a >= 0, b < 0
	; a < 0, b >= 0
	pop eax
	dec eax
	mov byte[eax], NEGATIVE
	push eax	

%%end:
	pop eax	
	mov dword[%8], eax
	
%endmacro

%endif
