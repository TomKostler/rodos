#pragma once
#include "asm_defines.h"


// Calls the SVC for the partition switch with the parameter in a r0,
// such that the id can be read out again in the SVC call
static inline void svc_partition_switch(unsigned int image_id) {
    asm volatile(
        "mov r0, %[id] \n\t"
        "svc %[imm]    \n\t"
        : 
        : [id] "r" (image_id),
          [imm] "I" (SWI_PARTITION_SWITCH)
        : "r0", "memory"
    );
}


// Calls the SVC for switching through all partitions directly after boot of one partition
// after initializing the hardware
static inline void svc_boot_partition_switch() {
    asm volatile(
        "svc %[imm]    \n\t"
        : 
        : [imm] "I" (SWI_PARTITION_SWITCH_BOOT)
        : "memory"
    );
}