	.text
	.globl force_gc
	.extern force_gc
force_gc:
	mov %rsp, %rdi
	jmp walkStack
