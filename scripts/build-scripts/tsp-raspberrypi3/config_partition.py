"""
Configuration file for the multi-partition RODOS on Raspberry Pi 3.

This module defines the memory layout and source files for each partition. 
It is used by:
1. "generate_linkerscripts.py" to create memory-specific .ld files.
2. "compile_partition_images.py" to orchestrate the build and naming process.

Key Parameters:
    - partition_source: The C++ entry point for the partition.
    - partition_image_name: The filename of the resulting binary image.
    - ram_origin: The physical start address in RAM.
    - ram_length: Total RAM allocated to this partition (e.g., "250M").
    - shared_origin: Start address of the shared memory region.
    - shared_length: Size of the shared memory region.
    - next_image_address: The RAM address of the next partition to boot (0x0 to
      stop).
    - partition_switch_interval_ticks: The number of hardware timer interrupts 
    that must occur before a partition switch is triggered.
"""

partition_configs = [
    {
        "partition_source": "partition_0.cpp",
        "partition_image_name": "kernel_partition_0.img",
        "ram_origin": "0x00100000",
        "ram_length": "250M", 
        "shared_origin": "0x30000000",
        "shared_length": "4K",
        "next_image_address": "0x0FB00000",
        "partition_switch_interval_ticks": "100"
    },
    {
        "partition_source": "partition_1.cpp",
        "partition_image_name": "kernel_partition_1.img",
        "ram_origin": "0x0FB00000",
        "ram_length": "250M",
        "shared_origin": "0x30000000",
        "shared_length": "4K",
        "next_image_address": "0x1F500000",
        "partition_switch_interval_ticks": "100"
    },
    {
        "partition_source": "partition_2.cpp",
        "partition_image_name": "kernel_partition_2.img",
        "ram_origin": "0x1F500000",
        "ram_length": "250M",
        "shared_origin": "0x30000000",
        "shared_length": "4K",
        "next_image_address": "0x0",
        "partition_switch_interval_ticks": "100"
    }
]



