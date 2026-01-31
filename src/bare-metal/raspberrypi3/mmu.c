/**
 * @file mmu.c
 * @date 2016/10/24
 * @author Michael Zehrer
 *
 */
#include <stdint.h>
#include <stdbool.h>

#include "include/bcm2837.h"
#include "include/mmu.h"
#include "include/asm_defines.h"
#include "partitionContext.h"
#include "partitions_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// Defined by the linker script
extern uint32_t _mmu_level1_table_start_[4096 * 4];


extern uint32_t __image_link_base__;
extern uint32_t __ram_end__;
extern uint32_t __shared_ctx_base__;
extern uint32_t __image_index__;
extern uint32_t __image_count__;
extern uint32_t partition_base_address;

extern PartitionEntry __partition_table_start__[];


void enable_mmu() {
    __asm volatile(
      // (TLBIALL) TLB Invalidate All entries (value in r0 is ignored)
      "mcr p15, 0, r0, c8, c7, 0\n\t"
      // read the System Control Register (SCTLR, page 4005 - ARMv8 Reference Manual)
      "mrc p15, 0, r0, c1, c0, 0\n\t"
      // ... set the M-bit ...
      "orr r0, r0, #" STR(SCTLR_M) "\n\t"
                                   // ... and write it back
                                   "mcr p15, 0, r0, c1, c0, 0\n\t"
      :
      :
      : "r0", "memory");
}

void disable_mmu() {
    __asm volatile(
      // read the System Control Register (SCTLR, page 4005 - ARMv8 Reference Manual)
      "mrc p15, 0, r0, c1, c0, 0\n\t"
      // ... clear the M-bit ...
      "bic r0, r0, #" STR(SCTLR_M) "\n\t"
                                   // ... and write it back
                                   "mcr p15, 0, r0, c1, c0, 0\n\t"
      :
      :
      : "r0", "memory");
}


void createPageTableEntries(uint32_t physical_adr, uint32_t virtual_adr, uint32_t length, bool cacheable, bool read_only, bool execute_never, bool no_access) {
    mmu_level1_section_t section;
    section.value = 0;


    // Only init page table entries if access is allowed for the partition
    if(!no_access) {
        section.bits._zero  = 0;
        section.bits._one   = 1;
        section.bits.domain = 0;

        if(cacheable) {
            // Outer and Inner Write-Back, no Write-Allocate
            section.bits.b = 1;
            section.bits.c = 1;
        }

        // Access Permission Bits
        section.bits.ap1_0 = 3;                 // Access Permitted for User & Privileged
        section.bits.ap2   = read_only ? 1 : 0; // Read/Write or ReadOnly Access

        // Execute Never Bit (XN)
        section.bits.xn = execute_never ? 1 : 0;
    }


    uint32_t physical_base = physical_adr >> 20;
    uint32_t start_section = virtual_adr >> 20;


    if(length == 0) return;

    // (virtual_adr + length - 1) gives the address of the last byte.
    uint32_t end_section = (virtual_adr + length - 1) >> 20;
    uint32_t entries     = end_section - start_section + 1;

    for(uint32_t i = 0; i < entries; ++i) {
        if(!no_access) { section.bits.base_address = (physical_base + i) & 0x0FFFu; }
        // Use loop index to increment virtual section index
        _mmu_level1_table_start_[start_section + i] = section.value;
    }
}

// Is called for each image/partition when they are first booted up
void init_mmu_for_current_partition() {
    // Reset Page Table. If the MMU tries to access an address that was not explicitly mapped,
    // Translation Fault is triggered
    for(int i = 0; i < 4096; i++) { _mmu_level1_table_start_[i] = 0; }


    // Set Domain to Client (01), since we need to set access rights (with register DACR)
    __asm volatile("mov r0, #1\n\t"
                   "mcr p15, 0, r0, c3, c0, 0\n\t"
                   :
                   :
                   : "r0");


    // Explicitly map the memory needed for the current partition
    uint32_t partition_start  = (uint32_t)&__image_link_base__;
    uint32_t partition_end    = (uint32_t)&__ram_end__;
    uint32_t partition_length = partition_end - partition_start;

    // Enable RWX for the current partition
    createPageTableEntries(partition_start, partition_start, partition_length, true, false, false, false);


    // Map the shared memory between partitions as RW, but not X
    uint32_t shared_base = (uint32_t)&__shared_ctx_base__;
    createPageTableEntries(shared_base, shared_base, 0x100000, true, false, true, false); // 1MB


    // Map the peripherals as RW, non-execute and non-cacheable for IO
    createPageTableEntries(PERIPHERALS_BASE, PERIPHERALS_BASE, PERIPHERALS_SIZE, false, false, true, false);
    createPageTableEntries(ARM_LOCAL_BASE, ARM_LOCAL_BASE, 0x00100000, false, false, true, false);


    // -------------------------------------------------------------------
    // Enable MMU
    // -------------------------------------------------------------------
    uintptr_t table_addr = (uintptr_t)_mmu_level1_table_start_;
    // Since table is (4096 * 4 Bytes / 0x4000) large
    for(uint32_t i = 0; i < 0x4000; i += 32) { // 32 Bytes = Cache Line Size
        // Clean and Invalidate Data Cache
        __asm volatile("mcr p15, 0, %0, c7, c14, 1" : : "r"(table_addr + i));
    }

    __asm volatile("dsb\n\t"
                   "isb\n\t"
                   "mcr p15, 0, r0, c7, c5, 0" // Invalidate I-Cache
                   :
                   :
                   : "r0");

    // Flush TLB
    __asm volatile("mcr p15, 0, r0, c8, c7, 0" : : : "r0");

    // Load Table Base Register
    __asm volatile("mcr p15, 0, %0, c2, c0, 0" : : "r"(table_addr));

    enable_mmu();
}

// Only flush specific entries for better performance
static void flush_page_table_entries(uint32_t vaddr, uint32_t length) {
    uint32_t  startIdx = vaddr >> 20;
    uint32_t  endIdx   = (vaddr + length - 1) >> 20;
    uintptr_t table    = (uintptr_t)_mmu_level1_table_start_;

    // Loop through every affected 1MB section in the Page Table
    for(uint32_t i = startIdx; i <= endIdx; ++i) {
        uintptr_t entry = table + (i * 4);
        // Push the entry from CPU Cache to RAM and invalidate the cache line
        __asm volatile("mcr p15, 0, %0, c7, c14, 1" : : "r"(entry));
    }
    __asm volatile("dsb");
}


// Is called in the old partition shortly before switching to the new partition (in partitionManager)
// Configures the MMU such that the new Partition can run accordingly
void partition_switch_mmu_caller() {

    uint32_t next_partition_start = partition_base_address;
    uint32_t num_partitions = (uint32_t)&__image_count__;
    uint32_t next_partition_len;
    for(uint32_t i = 0; i < num_partitions; i++) {
        if(partition_base_address == __partition_table_start__[i].start_addr) { next_partition_len = __partition_table_start__[i].length; }
    }

    // Set the new partition to RWX
    createPageTableEntries(next_partition_start, next_partition_start, next_partition_len, true, false, false, false);
    flush_page_table_entries(next_partition_start, next_partition_len);


    // Update the MMU by flushing the I-Cache and TLB
    __asm volatile("mcr p15, 0, r0, c7, c5, 0" : : : "r0");
    __asm volatile("mcr p15, 0, r0, c8, c7, 0" : : : "r0");
    __asm volatile("dsb\n\tisb");
}

// Is called in the partition that was switched to
// Configures the MMU to allow no access to any old partitions anymore
void partition_switch_mmu_callee() {
    uint32_t current_index = (uint32_t)&__image_index__;
    uint32_t num_partitions = (uint32_t)&__image_count__;


    // Set every partition that is not used to no access at all, except for the current (the new) one
    for(uint32_t i = 0; i < num_partitions; i++) {
        if(i == current_index) { continue; }

        uint32_t other_start = __partition_table_start__[i].start_addr;
        uint32_t other_len = __partition_table_start__[i].length;

        createPageTableEntries(other_start, other_start, other_len, true, true, false, true);
        flush_page_table_entries(other_start, other_len);
    }


    // Update the MMU by flushing the I-Cache and TLB
    __asm volatile("mcr p15, 0, r0, c7, c5, 0" : : : "r0");
    __asm volatile("mcr p15, 0, r0, c8, c7, 0" : : : "r0");
    __asm volatile("dsb\n\tisb");
}


#ifdef __cplusplus
} // end extern "C"
#endif
