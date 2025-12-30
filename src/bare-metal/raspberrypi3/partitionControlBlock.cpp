#include <stdint.h>


namespace RODOS {
    void xprintf(const char* fmt, ...);
}


extern "C" {


void asm_debug(uint32_t id) {
    // Aufruf mit Namespace-Qualifizierer
    RODOS::xprintf("\n[ASM] Checkpoint: 0x%x\n", id);
}






struct PartitionCtx {
	// Registers without a mode
	uint32_t VBAR;    // Vector Base Address Register
	uint32_t SCTLR;   // System Control Register
	uint32_t TTBR0;   // Translation Table Base
	// uint32_t DACR;    // Domain Access Control => Not needed since identity mapping in MMU used
	


	uint32_t image_link_base;




	// User Mode registers
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t r12;
	uint32_t r13; // Stack Pointer user mode
	uint32_t r14; // Lr
	uint32_t r15; // PC

	uint32_t CPSR;


	// SVC registers (rest of registers <13 is shared)
	uint32_t r13_svc; // Stack Pointer SVC
	uint32_t r14_svc; // Lr SVC Mode
	
	uint32_t SPSR_svc;


	// IRQ registers (rest of registers <13 is shared)
	uint32_t r13_irq; // Stack Pointer IRQ
	uint32_t r14_irq; // Lr IRQ Mode
	
	uint32_t SPSR_irq;


	// FIQ registers (rest of registers <8 is shared)
	uint32_t r8_fiq;
	uint32_t r9_fiq;
	uint32_t r10_fiq;
	uint32_t r11_fiq;
	uint32_t r12_fiq;
	uint32_t r13_fiq; // Stack Pointer FIQ
	uint32_t r14_fiq; // Lr FIQ Mode
	
	uint32_t SPSR_fiq;




	// Abort registers (rest of registers <13 is shared)
	uint32_t r13_abt; // Stack Pointer Abort mode
	uint32_t r14_abt; // Lr Abort Mode
	
	uint32_t SPSR_abt;



	// Undefined registers (rest of registers <13 is shared)
	uint32_t r13_und; // Stack Pointer undefined mode
	uint32_t r14_und; // Lr undefined Mode

	uint32_t SPSR_und;


};

__attribute__((section(".partition_ctx"), used, aligned(4)))
volatile PartitionCtx g_partition_ctx = {};

}
