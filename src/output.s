	.text
	.def	 @feat.00;
	.scl	3;
	.type	0;
	.endef
	.globl	@feat.00
.set @feat.00, 0
	.file	"main.kng"
	.def	 test_anon_fn;
	.scl	2;
	.type	32;
	.endef
	.globl	test_anon_fn
	.p2align	4, 0x90
test_anon_fn:
	retq

	.def	 main;
	.scl	2;
	.type	32;
	.endef
	.globl	main
	.p2align	4, 0x90
main:
	xorl	%eax, %eax
	retq

