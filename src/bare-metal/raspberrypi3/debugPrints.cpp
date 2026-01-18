#include <stdint.h>
#include "partitionContext.h"


namespace RODOS {
void xprintf(const char* fmt, ...);
}


extern "C" {

void asm_debug(uint32_t id) { RODOS::xprintf("\n[ASM] Checkpoint: 0x%x\n", id); }

void debug_log(const char* msg) { RODOS::xprintf("%s", msg); }
}
