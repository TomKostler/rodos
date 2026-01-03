 #include "include/asm_defines.h"

.extern g_partition_ctx

// Reserved Data for the changing image base address of the partition to switch to
.section .data
.align 4
partition_base_address: 
    .word 0x00000000


.section ".text"

.globl partition_switch_boot
.type  partition_switch_boot, %function

.globl switch_partition_caller
.type  switch_partition_caller, %function

.globl switch_partition_callee
.type  switch_partition_callee, %function

.extern asm_debug @ <--- @TODO: Delete if no Debug prints are needed anymore
.extern __next_image_start__




/** Partition Switch
 * Function is called in SVC mode
*/
partition_switch_boot:
    
    push    {r2}
    mov     r2, lr // Save return address to r2
    bl save_partition_context

    // Save the correct PC (PC of label boot_resume, otherwise cyclic calls 
    // with the jump)
    ldr     r12, =g_partition_ctx
    ldr     r1, =boot_resume
    str     r1, [r12, #76]


    /* One-way chainload: jump to image with starting address defined in current linkerscript */
    ldr     r0, =__next_image_start__
    bx      r0



boot_resume:
    bx      r2




/*
 * Save the complete context of the current image
 * For this, General Purpose Registers (r0-r14) are saved
*/
save_partition_context:
    cpsid   if // Deactivates IRQ's and Fast IRQ's

    @ pop     {r3} /* => Since r2, r3 were saved in switch_partition_caller and r12 before */

    /* Symbol g_partition_ctx points to a struct already allocated in 
      memory containing all necessary data for the complete context of an image */
    @ push    {r12} /* => Such that r12 is not overwritten before saving it in the struct*/
    ldr     r12, =g_partition_ctx


    /* Save User/System Registers r0-r14, with the needed offset for the struct g_partition_ctx */
    add     r1, r12, #16
    stmia   r1, {r0-r7}^
    add     r1, r12, #48
    stmia   r1, {r8-r14}^


    /* Save registers needed for system config 
        VBAR (Vector Base Address Register => Stores Baseaddress of exception vector table)
        SCTLR (System Control Register => MMU, Caches etc.)
        TTBR0 (Translation Table Base Register 0 => Page Tables etc.)
     */
    mrc     p15, 0, r0, c12, c0, 0      @ VBAR
    str     r0, [r12, #0]
    mrc     p15, 0, r0, c1, c0, 0       @ SCTLR
    str     r0, [r12, #4]
    mrc     p15, 0, r0, c2, c0, 0       @ TTBR0
    str     r0, [r12, #8]

    /* Save the image_link_base */
    ldr     r0, =__image_link_base__
    str     r0, [r12, #12]


    /* Save the User PC that's was stored in r2 before */
    str     r2, [r12, #76]

    /* Since the User CPSR (Current Program Status Register) is saved in a svc 
    to the SPSR, => save the SPSR to get the CPSR */
    mrs     r3, spsr
    str     r3, [r12, #80]

    /* Save the current SVC SP/LR/SPSR (Offsets 84,88,92) */
    mov     r4, sp
    str     r4, [r12, #84]              @ SVC SP
    str     r2, [r12, #88]              @ SVC LR
    str     r3, [r12, #92]              @ SVC SPSR

    /* Save SVC CPSR */
    mrs     r5, cpsr




    @ ==============================================================================
    @  SAVE UNDEFINED FIQ REGISTERS
    @ ==============================================================================
    /* => Since we are in priviledged SVC mode already, we can read the
        banked registers directly without the need to switch the mode first */ 
    mrs     r0, SP_IRQ
    mrs     r1, LR_IRQ
    mrs     r2, SPSR_IRQ
    str     r0, [r12, #96]
    str     r1, [r12, #100]
    str     r2, [r12, #104]
    @ ==============================================================================



    @ ==============================================================================
    @  SAVE UNDEFINED FIQ REGISTERS
    @ ==============================================================================
    mov     r7, r12 @ Save address of partitionCtx into non-banked registers
    
    /* Change mode to FIQ */
    mrs     r0, cpsr
    mov     r1, r0
    bic     r0, r0, #0x1F
    orr     r0, r0, #0x11 @ FIQ Mode == 0x11
    msr     cpsr_c, r0
    
    /* Save registers of FIQ mode */
    str     r8,  [r7, #108]     @ r8_fiq
    str     r9,  [r7, #112]     @ r9_fiq
    str     r10, [r7, #116]     @ r10_fiq
    str     r11, [r7, #120]     @ r11_fiq
    str     r12, [r7, #124]     @ r12_fiq
    str     sp,  [r7, #128]     @ sp_fiq
    str     lr,  [r7, #132]     @ lr_fiq
    mrs     r0, spsr
    str     r0,  [r7, #136]     @ spsr_fiq
    
    msr     cpsr_c, r1 @ Change mode back to SVC
    @ ==============================================================================



    @ ==============================================================================
    @  SAVE ABORT MODE REGISTERS
    @ ==============================================================================
    mov     r7, r12 @ Save address of partitionCtx into non-banked registers
    
    /* Change mode to ABORT */
    mrs     r0, cpsr
    mov     r1, r0
    bic     r0, r0, #0x1F
    orr     r0, r0, #0x17 @ ABORT-Mode == 0x17
    msr     cpsr_c, r0
    
    /* Save registers of ABORT mode */
    str     sp,  [r7, #140]     @ sp_abt
    str     lr,  [r7, #144]     @ lr_abt
    mrs     r0, spsr            
    str     r0,  [r7, #148]     @ spsr_abt
    
    msr     cpsr_c, r1 @ Change mode back to SVC
    @ ==============================================================================



    @ ==============================================================================
    @  SAVE UNDEFINED MODE REGISTERS
    @ ==============================================================================
    mov     r7, r12 @ Save address of partitionCtx into non-banked registers
    
    mrs     r0, cpsr
    mov     r1, r0
    bic     r0, r0, #0x1F
    orr     r0, r0, #0x1B @ Undefined-Mode == 0x1B
    msr     cpsr_c, r0
    
    /* Save registers of UNDEFINED mode */
    str     sp,  [r7, #152]     @ sp_und
    str     lr,  [r7, #156]     @ lr_und
    mrs     r0, spsr            
    str     r0,  [r7, #160]     @ spsr_und
    
    msr     cpsr_c, r1 @ Change mode back to SVC
    @ ==============================================================================

    @ pop {r12, r2}
    bx lr







load_partition_context:
    cpsid   if // Deactivates IRQ's and Fast IRQ's

    /* Symbol g_partition_ctx points to a struct already allocated in 
      memory containing all necessary data for the complete context of an image */
    ldr     r12, =g_partition_ctx


    /* Load system config registers */
    ldr     r0, [r12, #0]
    mcr     p15, 0, r0, c12, c0, 0      @ VBAR
    ldr     r0, [r12, #4]
    mcr     p15, 0, r0, c1, c0, 0       @ SCTLR
    ldr     r0, [r12, #8]
    mcr     p15, 0, r0, c2, c0, 0       @ TTBR0


    /* Load the SVC SP/LR/SPSR with Offsets 84,88,92 */
    ldr     r4, [r12, #84]          @ r13_svc
    mov     sp, r4
    ldr     r2, [r12, #88]          @ r14_svc
    mov     lr, r2
    ldr     r3, [r12, #92]          @ SPSR_svc
    msr     spsr_cxsf, r3




    @ ==============================================================================
    @  RESTORE IRQ MODE REGISTERS
    @ ==============================================================================
    mov     r7, r12 @ Save address of partitionCtx into non-banked registers
    
    @ Switch to IRQ Mode
    mrs     r0, cpsr
    mov     r1, r0
    bic     r0, r0, #0x1F
    orr     r0, r0, #0x12           @ IRQ-Modus
    msr     cpsr_c, r0
    
    @ Restore registers
    ldr     sp,  [r7, #96]          @ sp_irq
    ldr     lr,  [r7, #100]         @ lr_irq
    ldr     r0,  [r7, #104]         @ spsr_irq
    msr     spsr_cxsf, r0
    
    msr     cpsr_c, r1              @ Back to SVC mode
    @ ==============================================================================




    @ ==============================================================================
    @  RESTORE FIQ MODE REGISTERS
    @ ==============================================================================
    mov     r7, r12 @ Save address of partitionCtx into non-banked registers
    
    @ Switch to FIQ Mode
    mrs     r0, cpsr
    mov     r1, r0
    bic     r0, r0, #0x1F
    orr     r0, r0, #0x11           @ FIQ-Modus
    msr     cpsr_c, r0
    
    @ Restore registers
    ldr     r8,  [r7, #108]         @ r8_fiq
    ldr     r9,  [r7, #112]         @ r9_fiq
    ldr     r10, [r7, #116]         @ r10_fiq
    ldr     r11, [r7, #120]         @ r11_fiq
    ldr     r12, [r7, #124]         @ r12_fiq
    ldr     sp,  [r7, #128]         @ sp_fiq
    ldr     lr,  [r7, #132]         @ lr_fiq
    ldr     r0,  [r7, #136]         @ spsr_fiq
    msr     spsr_cxsf, r0
    
    msr     cpsr_c, r1 @ Back to SVC
    @ ==============================================================================



    @ ==============================================================================
    @  RESTORE ABORT MODE REGISTERS
    @ ==============================================================================
    mov     r7, r12 @ Save address of partitionCtx into non-banked registers
    
    @ Switch to ABORT Mode
    mrs     r0, cpsr
    mov     r1, r0
    bic     r0, r0, #0x1F
    orr     r0, r0, #0x17           @ Abort-Mode
    msr     cpsr_c, r0
    
    @ Restore registers
    ldr     sp,  [r7, #140]         @ sp_abt
    ldr     lr,  [r7, #144]         @ lr_abt
    ldr     r0,  [r7, #148]         @ spsr_abt
    msr     spsr_cxsf, r0
    
    msr     cpsr_c, r1 @ Back to SVC
    @ ==============================================================================



    @ ==============================================================================
    @  RESTORE UNDEFINED MODE REGISTERS
    @ ==============================================================================
    mov     r7, r12 @ Save address of partitionCtx into non-banked registers
    
    @ Switch to UNDEFINED Mode
    mrs     r0, cpsr
    mov     r1, r0
    bic     r0, r0, #0x1F
    orr     r0, r0, #0x1B           @ Undefined-Mode
    msr     cpsr_c, r0
    
    @ Restore registers
    ldr     sp,  [r7, #152]         @ sp_und
    ldr     lr,  [r7, #156]         @ lr_und
    ldr     r0,  [r7, #160]         @ spsr_und
    msr     spsr_cxsf, r0
    
    msr     cpsr_c, r1 @ Back to SVC mode
    @ ==============================================================================


    

    ldr     r12, =g_partition_ctx        @ Reload base address

    /* 1. Restore User CPSR (Offset 80) into SPSR */
    ldr     r0, [r12, #80]
    msr     spsr_cxsf, r0

    /* 2. Restore User PC (Offset 76) into LR */
    /* We load it into LR because 'movs pc, lr' will later move it to PC */
    ldr     lr, [r12, #76]

    /* 3. Restore User SP and LR (Offset 68) */
    add     r12, r12, #68
    ldmia   r12, {sp, lr}^
    sub     r12, r12, #68

    /* 4. Restore User r0-r7 (Offset 16) */
    add     r12, r12, #16
    ldmia   r12, {r0-r7}^ @ r12 is now Base+16

    /* 5. Restore User r8-r11 (Offset 48) */
    /* Offset 48 - 16 = 32 */
    add     r12, r12, #32
    ldmia   r12, {r8-r11}^ @ r12 is now Base+48

    /* 6. Restore User r12 (Offset 64) */
    /* Offset 64 - 48 = 16 */ 
    ldr     r12, [r12, #16]

    movs    pc, lr






/*
 * Handles the complete partition switch by saving the current partition/image state 
 * and branching to the new image
*/
switch_partition_caller:
    @ --- @TODO: DEBUG 1: Start Caller ---
    push {r0-r12, lr}
    mov r0, #0x1            @ ID 1
    bl asm_debug
    pop {r0-r12, lr}
    @ -----------------------------

    /* Retrieve the argument of the SWI interrupt call indicating the link address of the partition */
    pop {r0} 

    /* save registers that are used in this function to the stack so that the 
    whole context is correctly saved */
    @ push {r2, r3}


    ldr r3, =partition_base_address
    str r0, [r3]

    

    /* Save PC to r2, otherwise it gets overwritten */
    mov r2, lr


    bl save_partition_context


    @ --- @TODO: DEBUG 2: After Save ---
    push {r0-r12, lr}
    mov r0, #0x2            @ ID 2
    bl asm_debug
    pop {r0-r12, lr}
    @ --------------------------




    /* To avoid cyclic call => Set the saved PC to a resume label */
    ldr     r12, =g_partition_ctx
    ldr     r0, [r12, #76] @ Load the saved PC from offset 76
    ldr     r1, =caller_resume  @ Load address of label resume
    str     r1, [r12, #76] @ Overwrite saved PC



    /* Write the Magic number 0xCAFEBABE into r0 such that the
     next image can check whether it was reached by a context switch*/
    ldr r0, =0xCAFEBABE
    
    // Load the reset address of the partition to jump to, specified when calling the SWI
    ldr r3, =partition_base_address
    ldr r1, [r3]
    bx  r1

caller_resume:
    bx r2





/*
 * Entry point of the image that should be switched to directly after the reset.
 * Handles the loading of a partition.
*/
switch_partition_callee:
    @ --- DEBUG 3: Entry Callee ---
    ldr sp, =__stack_svc_top__ @ Since we have no stack, we need to set it in order to write to UART
    push {r0-r12, lr}
    mov r0, #0x3            @ ID 3
    bl asm_debug
    pop {r0-r12, lr}
    @ -------------------------------


    bl load_partition_context





.size  partition_switch_boot, .-partition_switch_boot
.size  save_partition_context, .-save_partition_context
.size  load_partition_context, .-load_partition_context