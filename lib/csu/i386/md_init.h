/* $OpenBSD: md_init.h,v 1.6 2015/09/01 05:40:06 guenther Exp $ */

/*-
 * Copyright (c) 2001 Ross Harvey
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the NetBSD
 *      Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define MD_SECT_CALL_FUNC(section, func) \
	__asm (".section "#section", \"ax\"\n"	\
	"	call " #func "\n"		\
	"	.previous")

/*
 * Align is after because we want the function to start at the first
 * address of the section, but overall we want the section to be
 * aligned by the align amount.
 */
#define MD_SECTION_PROLOGUE(sect, entry_pt)	\
	__asm (					\
	".section "#sect",\"ax\",@progbits	\n" \
	"	.globl " #entry_pt "		\n" \
	"	.type " #entry_pt ",@function	\n" \
	#entry_pt":				\n" \
	"	.align 16			\n" \
	"	pushl	%ebp			\n" \
	"	movl	%esp,%ebp		\n" \
	"	andl	$~15,%esp		\n" \
	"	.previous")


#define MD_SECTION_EPILOGUE(sect)		\
	__asm (					\
	".section "#sect",\"ax\",@progbits	\n" \
	"	leave				\n" \
	"	ret				\n" \
	"	.previous")


#define	MD_CRT0_START				\
	__asm(					\
	".text					\n" \
	"	.align	4			\n" \
	"	.globl	__start			\n" \
	"	.globl	_start			\n" \
	"_start:				\n" \
	"__start:				\n" \
	"	movl	%esp,%ebp		\n" \
	"	andl	$~15,%esp	# align stack\n" \
	"	pushl	%edx		# cleanup\n" \
	"	movl	0(%ebp),%eax		\n" \
	"	leal	8(%ebp,%eax,4),%ecx	\n" \
	"	leal	4(%ebp),%edx		\n" \
	"	pushl	%ecx			\n" \
	"	pushl	%edx			\n" \
	"	pushl	%eax			\n" \
	"	xorl	%ebp,%ebp	# mark deepest stack frame\n" \
	"	call	___start		\n" \
	"	.previous")

#define	MD_RCRT0_START				\
	__asm(					\
	".text					\n" \
	"	.align	4			\n" \
	"	.globl	__start			\n" \
	"	.globl	_start			\n" \
	"_start:				\n" \
	"__start:				\n" \
	"	movl	%esp,%eax	# save SP for dl_boot_bind \n" \
	"	subl	$16*4,%esp 	# allocate dl_data \n" \
	"	pushl	$0	# push 0 for dynamicp (unused on i386) \n" \
	"	movl	%esp,%ebx		\n" \
	"	pushl	%ebx		# push dl_data for dl_boot_bind \n" \
	"					\n" \
	"	mov	%eax, %esi	# save stack for ___start \n" \
	"	pushl	%eax		# push saved SP for dl_boot_bind \n" \
	"					\n" \
	"	call	_dl_boot_bind@PLT # _dl_boot_bind(sp,dl_data,0) \n" \
	"	addl	$3*4,%esp	# pop args \n" \
	"					\n" \
	"	movl	%esi,%ebp		\n" \
	"	andl	$~15,%esp	# align stack\n" \
	"	pushl	$0		# cleanup\n" \
	"	movl	0(%ebp),%eax	# argc	\n" \
	"	leal	8(%ebp,%eax,4),%ecx #envp \n" \
	"	leal	4(%ebp),%edx	# argv	\n" \
	"	pushl	%ecx			\n" \
	"	pushl	%edx			\n" \
	"	pushl	%eax			\n" \
	"	xorl	%ebp,%ebp	# mark deepest stack frame\n" \
	"	call	___start	# ___start(argc,argv,envp,0) \n" \
	"					\n" \
	".text					\n" \
	"	.align	4			\n" \
	"	.globl	_dl_printf		\n" \
	"	.type	_dl_printf,@function	\n" \
	"_dl_printf:				\n" \
	"	ret				\n" \
	"					\n" \
	".text					\n" \
	"	.align	4			\n" \
	"	.globl	_dl_exit		\n" \
	"	.type	_dl_exit,@function	\n" \
	"_dl_exit:				\n" \
	"	mov	$1, %eax		\n" \
	"	int	$0x80			\n" \
	"	ret				\n" \
	"	.previous")

#include <sys/syscall.h>
#define	MD_DISABLE_KBIND						\
	do {								\
		register long syscall_num __asm("eax") = SYS_kbind;	\
		__asm volatile("pushl $0; push %%eax; int $0x80;"	\
		    " addl $8, %%esp" : "+a" (syscall_num) :		\
		    : "edx", "cc");					\
	} while (0)
