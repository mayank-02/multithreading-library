#ifndef _MANGLE_H_
#define _MANGLE_H_

#ifdef __x86_64__
	/* Pointer mangling for 64 bit Intel architecture */
	#define JB_SP 6
	#define JB_PC 7
	static long int mangle(long int p) {
		long int ret;
		asm(" mov %1, %%rax;\n"
			" xor %%fs:0x30, %%rax;"
			" rol $0x11, %%rax;"
			" mov %%rax, %0;"
			: "=r"(ret)
			: "r"(p)
			: "%rax"
		);
		return ret;
	}
#else
	/* Pointer mangling for 32 bit Intel architecture */
	#define JB_SP 4
	#define JB_PC 5
	static unsigned int mangle(unsigned int addr) {
		unsigned int ret  ;
		asm volatile("xor %%gs:0x18,%0\n"
					 "rol $0x9,%0\n"
					 : "=g" (ret)
					 : "0" (addr));
		return ret;
	}
#endif

#endif