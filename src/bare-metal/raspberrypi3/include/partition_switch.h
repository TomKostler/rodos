#pragma once
#include "asm_defines.h"



static inline void svc_partition_switch(void) {
    asm volatile("svc %[imm]" :: [imm] "I" (SWI_PARTITION_SWITCH) : "memory");
}
