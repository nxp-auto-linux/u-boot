#include <common.h>
#include <asm/io.h>

int arch_early_init_r(void) {
	memset(CPU_RELEASE_ADDR, 0, 4* sizeof(unsigned long));
	/*writel(0x0, CPU_RELEASE_ADDR); */
	smp_kick_all_cpus();
}
