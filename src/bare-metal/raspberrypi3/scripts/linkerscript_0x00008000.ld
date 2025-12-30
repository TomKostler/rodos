OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
SEARCH_DIR(.)

/* not needed by RODOS just for newlib */
_estack = 0x20020000;  
_Min_Heap_Size = 0x1000;
_Min_Stack_Size = 0x400;


MEMORY {
  ram (rwx)  : ORIGIN = 0x00008000, LENGTH = 511M

  /* Needed to share pointers to the stacks of different images for partition switching, put after every loaded image in the RAM */
  shared_memory_images (rw)   : ORIGIN = 0x30000000, LENGTH = 4K 
}

SECTIONS {
  .text : { 
    KEEP (*(.vectors .vectors.*))
    *(.text.startup)
    *(.text)
    *(.text*)
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.jcr)
    *(.vfp11_veneer)
    *(.gnu.linkonce.t*)
  } > ram

  __image_link_base__ = ORIGIN(ram);
  PROVIDE_HIDDEN(__image_count__ = 2);
  PROVIDE_HIDDEN(__image_index__ = 0);
  __shared_ctx_slot_size__ = 0x400; /* 1 Slot = 1 KiB (>= sizeof(PartitionCtx)=168B) */
  __shared_ctx_base__ = ORIGIN(shared_memory_images);
  __shared_ctx_addr__ = __shared_ctx_base__ + __image_index__ * __shared_ctx_slot_size__;



  .ARM.extab : {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > ram

  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } > ram

  .rodata : {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata.*)
    *(.gnu.linkonce.r*)
  } > ram

  .init_array :
  {
    . = ALIGN(4);
    PROVIDE (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    PROVIDE (__init_array_end = .);
  } > ram
  
  .fini_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(4);
  } > ram
  
  .data : {
    . = ALIGN(4);
    *(.data)
    *(.data*)
    *(.gnu.linkonce.d*)
  } > ram

  .bss : {
    . = ALIGN(4);
    __bss_start__ = .;
    *(.bss)
    *(.bss*)
    *(COMMON)

    /* The MMU page table must be 16KB aligned (see TTBR0 register) */
    . = ALIGN(0x4000);
    _mmu_level1_table_start_ = .;
    /* Only the first level page table is used.
       This gives us a total size of 4096 * 4B = 16 KB */
    . = . + 0x4000;
    _mmu_level1_table_end_ = .;

    __bss_end__ = .;
  } > ram

  PROVIDE ( end = . );
  PROVIDE ( _end = . );
  PROVIDE ( __end__ = . );

  ._user_heap_stack : {
    . = ALIGN(4);
    PROVIDE ( __heap_start__ = .);
    . = . + _Min_Heap_Size;
    PROVIDE ( __heap_end__ = .);
    . = . + _Min_Stack_Size;
    . = ALIGN(4);
  } > ram



  /* Shared section for the contexts of different partitions/images */
  .partition_ctx __shared_ctx_addr__ (NOLOAD) : AT(__shared_ctx_addr__) {
    . = ALIGN(4);
    KEEP(*(.partition_ctx))
    . = ALIGN(__shared_ctx_slot_size__);
  } > shared_memory_images




  /* Place the Stacks at the end of the available RAM, important for RAM-Partitioning
   * System stack first, since it grows dynamically and can get large because of user programs (=> Not overriding other stacks) 
   */
  __ram_end__        = ORIGIN(ram) + LENGTH(ram);
  __stack_svc_top__  = ALIGN(__ram_end__, 16);
  __stack_irq_top__  = __stack_svc_top__ - 0x5000;
  __stack_sys_top__  = __stack_irq_top__ - 0x4000;





  /DISCARD/ : {
    *(.comment)
    *(.note*)
    *(.eh_frame)
  }
}

