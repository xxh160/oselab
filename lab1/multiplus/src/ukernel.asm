SECTION .text
global ukernel

ukernel:
	int 80H
	ret
