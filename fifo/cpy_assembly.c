#ifndef __CPYASSMBLY__
#define __CPYASSMBLY__ 1
/**
 * Collection of code for a "streaming" memcpy, currently set up only
 * for x86, will add ARM soon.
 */

//TODO, comment all below code

typedef struct {
     uint32_t
         CF      :  1,
                 :  1,
         PF      :  1,
                 :  1,
         AF      :  1,
                 :  1,
         ZF      :  1,
         SF      :  1,
         TF      :  1,
         IF      :  1,
         DF      :  1,
         OF      :  1,
         IOPL    :  2,
         NT      :  1,
                 :  1,
         RF      :  1,
         VM      :  1,
         AC      :  1,
         VIF     :  1,
         VIP     :  1,
         ID      :  1,
                 : 10;
} EFlags;


typedef struct {
   uint32_t eax, ebx, ecx, edx;
} Reg;

enum feature_levels {
	FL_NONE,
	FL_MMX,
	FL_SSE2,
	FL_AVX
};


#define    CPUID_BASIC     0x0
#define    CPUID_LEVEL1    0x1

void zero_registers (Reg *in)
{
	in->eax = 0x0;
	in->ebx = 0x0;
	in->ecx = 0x0;
	in->edx = 0x0;
}


/**
 * get_cpuid - sets eax, ebx, ecx, edx values in struct Reg based on input_eax input
 */
void get_cpuid (Reg *input_registers, Reg *output_registers)
{
#if( __i386__ == 1 || __x86_64 == 1 )
   __asm__ volatile ("\
      movl  %[input_eax], %%eax     \n\
      movl  %[input_ecx], %%ecx     \n\
      cpuid                         \n\
      movl  %%eax,        %[eax]    \n\
      movl  %%ebx,        %[ebx]    \n\
      movl  %%ecx,        %[ecx]    \n\
      movl  %%edx,        %[edx]"
      :
      [eax] "=r"  (output_registers->eax),
      [ebx] "=r"  (output_registers->ebx),
      [ecx] "=r"  (output_registers->ecx),
      [edx] "=r"  (output_registers->edx)
      :
      [input_eax] "m" (input_registers->eax),
      [input_ecx] "m" (input_registers->ecx)
      :
      "eax","ebx","ecx","edx"
      );
#endif
}

int get_level0_data (unsigned int *max_level)
{
	Reg in, out;

	in.eax = CPUID_BASIC;
	get_cpuid(&in, &out);
	
	if (max_level)
		*max_level = out.eax;
		
	return 0;
}


int get_level1_data (unsigned int max_level, unsigned int *eax, 
	unsigned int *ecx, unsigned int *edx)
{
	Reg in, out;

	in.eax = CPUID_LEVEL1;
	get_cpuid(&in, &out);
	
	if (eax) *eax = out.eax;
	if (ecx) *ecx = out.ecx;
	if (edx) *edx = out.edx;
	
	return 0;
}

enum feature_levels get_highest_feature (unsigned int max_level)
{
	unsigned int ecx, edx;

	if (max_level < 1) {
		fprintf(stderr, "Error calling cpuid, cpuid not supported\n");
		exit(-1);
	}
	
	get_level1_data(max_level, NULL, &ecx, &edx);

	if (ecx & (1 << 28))
		return FL_AVX;

	if (edx & (1 << 26))
		return FL_SSE2;
		
	if (edx & (1 << 23))
		return FL_MMX;
			
	return FL_NONE;
}

      /*  	loop128%=:				\n\
			vmovdqu (%%rax), %%ymm0		\n\
			vmovdqu 32(%%rax), %%ymm1	\n\
			vmovdqu 64(%%rax), %%ymm2	\n\
			vmovdqu 96(%%rax), %%ymm3	\n\
			vmovdqu %%ymm0, (%%rbx)		\n\
			vmovdqu %%ymm1, 32(%%rbx)	\n\
			vmovdqu %%ymm2, 64(%%rbx)	\n\
			vmovdqu %%ymm3, 96(%%rbx)	\n\
			addq	$128, %%rax		\n\
			addq	$128, %%rbx		\n\
			subq	$128, %[SIZE]		\n\
			l128ctl%=:			\n\
			cmpq	$128, %[SIZE]		\n\
			jge	loop128%=		\n\
			jmp	l8ctl%=			\n\*/

inline 
void 
rb_write( unsigned char *dstp, 
          unsigned char *srcp, 
	       size_t size, 
          char feature_level )
{
	__asm__ volatile("\
			movq	%[in], %%rax		\n\
			movq	%[out], %%rbx		\n\
			cmpb	$1, %[fl]		\n\
			jl	l8ctl%=			\n\
			je	l32ctl%=		\n\
			movq	%%rbx, %%rdx		\n\
			andq	$15, %%rdx		\n\
			cmpq	$0, %%rdx		\n\
			je	l64ctl%=		\n\
			jmp 	l64uctl%=		\n\
		loop64%=:				\n\
			movdqu	(%%rax), %%xmm0		\n\
			movdqu	16(%%rax), %%xmm1	\n\
			movdqu	32(%%rax), %%xmm2	\n\
			movdqu	48(%%rax), %%xmm3	\n\
			movntdq	%%xmm0, (%%rbx)		\n\
			movntdq	%%xmm1, 16(%%rbx)	\n\
			movntdq	%%xmm2, 32(%%rbx)	\n\
			movntdq	%%xmm3, 48(%%rbx)	\n\
			addq	$64, %%rax		\n\
			addq	$64, %%rbx		\n\
			subq	$64, %[SIZE]		\n\
			l64ctl%=:			\n\
			cmpq	$64, %[SIZE]		\n\
			jge	loop64%=		\n\
			jmp	l32ctl%=		\n\
		loop64unalign%=:			\n\
			movq	(%%rax), %%mm0		\n\
			movq	8(%%rax), %%mm1		\n\
			movq	16(%%rax), %%mm2	\n\
			movq	24(%%rax), %%mm3	\n\
			movq	32(%%rax), %%mm4	\n\
			movq	40(%%rax), %%mm5	\n\
			movq	48(%%rax), %%mm6	\n\
			movq	56(%%rax), %%mm7	\n\
			movntq	%%mm0, (%%rbx)		\n\
			movntq	%%mm1, 8(%%rbx)		\n\
			movntq	%%mm2, 16(%%rbx)	\n\
			movntq	%%mm3, 24(%%rbx)	\n\
			movntq	%%mm4, 32(%%rbx)	\n\
			movntq	%%mm5, 40(%%rbx)	\n\
			movntq	%%mm6, 48(%%rbx)	\n\
			movntq	%%mm7, 56(%%rbx)	\n\
			addq	$64, %%rax		\n\
			addq	$64, %%rbx		\n\
			subq	$64, %[SIZE]		\n\
			l64uctl%=:			\n\
			cmpq	$64, %[SIZE]		\n\
			jge	loop64unalign%=		\n\
			jmp 	l32ctl%=		\n\
		loop32%=:				\n\
			movq	(%%rax), %%mm0		\n\
			movq	8(%%rax), %%mm1		\n\
			movq	16(%%rax), %%mm2	\n\
			movq	24(%%rax), %%mm3	\n\
			movntq	%%mm0, (%%rbx)		\n\
			movntq	%%mm1, 8(%%rbx)		\n\
			movntq	%%mm2, 16(%%rbx)	\n\
			movntq	%%mm3, 24(%%rbx)	\n\
			addq	$32, %%rax		\n\
			addq	$32, %%rbx		\n\
			subq	$32, %[SIZE]		\n\
			l32ctl%=:			\n\
			cmpq	$32, %[SIZE]		\n\
			jge	loop32%=		\n\
			jmp 	l8ctl%=			\n\
		loop8%=:				\n\
			movq	(%%rax), %%rcx		\n\
			movq	%%rcx, (%%rbx)		\n\
			addq	$8, %%rax		\n\
			addq	$8, %%rbx		\n\
			subq	$8, %[SIZE]		\n\
			l8ctl%=:			\n\
			cmpq	$8, %[SIZE]		\n\
			jge	loop8%=			\n\
			jmp 	l1ctl%=			\n\
		loop1%=:				\n\
			movb	(%%rax), %%cl		\n\
			movb	%%cl, (%%rbx)		\n\
			incq	%%rax			\n\
			incq	%%rbx			\n\
			decq	%[SIZE]			\n\
			l1ctl%=:			\n\
			cmpq	$1, %[SIZE]		\n\
			jge	loop1%=			\n\
			mfence"
			:
			:
			[in] "g" (srcp), 
			[out] "g" (dstp),			
			[SIZE] "m" (size),
			[fl] "g" (feature_level)
			:
			"mm0", "mm1", "mm2", "mm3", 
			"xmm0", "xmm1", "xmm2", "xmm3",
			"rax", "rbx", "rcx");	
}

#endif
