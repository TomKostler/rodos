 #include "include/asm_defines.h"

.section .partition_ctx
.align 4
.global g_partition_ctx
g_partition_ctx:
    .space 0x400



/* Reserved Data for the changing image base address of the partition to switch to */
.section .data
.align 4
.globl partition_base_address
partition_base_address: 
    .word 0x00000000


.section ".text"

.globl partition_switch_boot
.type  partition_switch_boot, %function

.globl switch_partition_caller
.type  switch_partition_caller, %function

.globl switch_partition_callee
.type  switch_partition_callee, %function

.globl switch_partition_from_irq_caller
.type  switch_partition_from_irq_caller, %function


.extern __next_image_start__
.extern partition_switch_mmu_caller
.extern partition_switch_mmu_callee

.extern save_partition_context
.extern load_partition_context





/* Partition Switch
 * Function is called in SVC mode
 */
partition_switch_boot:    
    push    {r2}
    mov     r2, lr @ Save return address to r2


    bl save_partition_context


    /* Save the correct PC (PC of label boot_resume, otherwise cyclic calls 
     * with the jump) 
     */
    ldr     r12, =g_partition_ctx
    ldr     r1, =boot_resume
    str     r1, [r12, #76]

    /* ----------------------------------------------------------------
     * TURN OFF MMU (before branch to different image/partition)
     * ----------------------------------------------------------------
     * 1. Data Cache Clean (und Invalidate) 
    */
    dsb
    mrc     p15, 0, r1, c1, c0, 0
    bic     r1, r1, #0x1000         @ I
    bic     r1, r1, #0x4            @ C
    bic     r1, r1, #0x1            @ M
    mcr     p15, 0, r1, c1, c0, 0
    isb
    mcr     p15, 0, r0, c8, c7, 0   @ TLBIALL
    dsb
    isb

    /* 2. Disable MMU & Caches */
    mrc     p15, 0, r1, c1, c0, 0
    bic     r1, r1, #0x5            @ Clear M (0x1) and C (0x4)
    mcr     p15, 0, r1, c1, c0, 0
    isb


    /* One-way chainload: jump to image with starting address defined in current linkerscript */
    ldr     r0, =__next_image_start__
    bx      r0

boot_resume:
    bx      r2





/*
 * Handles the complete partition switch by saving the current partition/image state 
 * and branching to the new image
 */
switch_partition_caller:
    /* Retrieve the argument of the SWI interrupt call indicating the link address of the partition */
    pop {r0} 

    ldr r3, =partition_base_address
    str r0, [r3]

    
    /* Save PC to r2, otherwise it gets overwritten */
    mov r2, lr


    bl save_partition_context


    /* To avoid cyclic call => Set the saved PC to a resume label */
    ldr     r12, =g_partition_ctx
    ldr     r0, [r12, #76]      @ Load the saved PC from offset 76
    ldr     r1, =caller_resume  @ Load address of label resume
    str     r1, [r12, #76]      @ Overwrite saved PC


    /* Set the correct MMU settings before the switch */
    bl      partition_switch_mmu_caller


    /* Write the Magic number 0xCAFEBABE into r0 such that the
     * next image can check whether it was reached by a context switch
     */
    ldr r0, =0xCAFEBABE

    
    /* Load the reset address of the partition to jump to, specified when calling the SWI */
    ldr r3, =partition_base_address
    ldr r1, [r3]
    bx  r1

caller_resume:
    bx r2




/*
 * Entry point of the image that should be switched to directly after the reset.
 * Handles the loading of the correct MMU settings and the loading of a partition.
*/
switch_partition_callee:
    bl partition_switch_mmu_callee
    bl load_partition_context




/*
 * Called when hardware timer interrupt fires => time slice is over => Partition switch
 * Works in IRQ mode (not SVC!)
 */
switch_partition_from_irq_caller:
    /* Save caller-saved registers, return address and CPU mode to the CURRENT IRQ stack */
    push    {r0-r3, r12, lr}
    
    mrs     r3, cpsr
    
    ldr     r1, =partition_base_address
    str     r0, [r1]
    
    /* Change mode to SVC (save_partition_context needs this mode!) */
    msr     cpsr_c, #0xD3
    push    {r3}
    
    bl      save_partition_context

    pop     {r3}
    
    /* set resume point */
    ldr     r12, =g_partition_ctx
    
    ldr     r1, =caller_resume_irq
    str     r1, [r12, #76]      @ Offset 76 = PC
    
    str     r3, [r12, #80]      @ Offset 80 = CPSR (korrekt gesicherter Wert)
    
    bl      partition_switch_mmu_caller

    
    /* Jump to new partition */
    ldr     r0, =0xCAFEBABE
    ldr     r3, =partition_base_address
    ldr     r1, [r3]
    bx      r1

caller_resume_irq:
    pop     {r0-r3, r12, pc}






.size  partition_switch_boot, .-partition_switch_boot
