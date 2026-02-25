# Time-Space-Partitioning (TSP) System for RODOS

> This repo acts an extension for **RODOS** *(Realtime Onboard Dependable Operating System)* on Raspberry Pi 3 Systems. It contains three sample applications that can be run as partitions completely isolated from each other with the help of the MMU. More partitions can be configured as needed.


#### Context Switching
**Context Switching** between the partitions is implemented as both Software Interrupts and Hardware Interrupts:

* **Hardware Interrupts**: Since hard real-time preemption is necessary in TSP systems, this extension makes use of the hardware timer on the Raspberry Pi to switch partitions every time a time slice expired.

* **Software Interrupts**: Can be triggered voluntarily by each application partition additional to the hardware timer interrupts. For this, specific svc functions are provided that handle the context switch.

#### Build Infrastructure
In order to simplify the build process regarding the partitions (adaptations of linker files, parsing config data etc.), build infrastructure through python scripts is provided. More on that in the section `Usage`.


#### RODOS Fork
The necessary changes to the RODOS operating system have been implemented on the branch `feature_partitioned_images` of the [rodos-fork](https://github.com/TomKostler/rodos/tree/feature_partitioned_images) linked in this repo.



<br>


## Usage

### Build infrastructure
In order to use multiple partitions in RODOS' main memory, build infrastructure is provided to simplify the creation of partitioned applications: 



* **`config_partition.py`**: Main config file for specifying key parameters within a list of dicts for every application that should run as a partition. 

	| Parameter | Description |
	| :--- | :--- |
	| partition_source | The C++ entry point for the partition. |
	| partition_image_name | The filename of the resulting binary image. |
	| ram_origin | The physical start address in RAM. |
	| ram_length | Total RAM allocated to this partition (e.g., "250M"). |
	| shared_origin | Start address of the shared memory region. |
	| shared_length | Size of the shared memory region. |
	| next_image_address | The RAM address of the next partition to boot (0x0 to stop). |
	| partition_switch_interval_ticks | The number of hardware timer interrupts that must occur before a partition switch is triggered. |

	This module is used by:

	1. `generate_linkerscripts.py` to create memory-specific .ld files.
	2. `compile_partition_images.py` to orchestrate the build and naming process.


* **`generate_linkerscripts.py`**:
This module handles the generation of custom linker scripts for multi-partition booting. It uses a template linkerscript file and a configuration list given by `config_partition.py` to calculate and write specific memory layouts (RAM origin, shared memory, and boot-chain addresses) for each partition.


* **`compile_partition_images.py`**:
This module is the main script for automating the build process for multiple Raspberry Pi 3 partitions. It iterates through partition configurations that are specified in the module `config_partition.py`, swaps linker scripts such that each partition is loaded into the correct part of memory, compiles source files using the RODOS build-python scripts and generates binary images for booting.


### How-To
1. **Configure Boot MicroSD-Card for Raspberry Pi 3**:
It must have at least one partition on it that is formatted with the FAT file system. Additionally it must contain the Raspbis [bootloader firmware](https://github.com/raspberrypi/firmware/tree/master/boot). More information on how to use RODOS on Raspberry Pi can be found [here](https://github.com/TomKostler/rodos/blob/feature_partitioned_images/doc/how-to-make-a-boot-SD-card-for-Raspberry.md).

2. Execute
    
    `cd rodos/scripts/build-scripts/tsp-raspberrypi3`

    `python3 compile_partition_images.py /path/to/folder/containing/rodos_application_partitions`


    This generates the RODOS application images specified in `config_partitions.py` that can be used as partitions.


3. Either set up a network boot over TFTP (recommended for faster development) or copy the generated images from `rodos/scripts/build-scripts/tsp-raspberrypi3` manually to the boot MicroSD-Card.

4. **uboot**:
Adjust uboot to load all partition images to the specified locations in RAM. This can look like this:

	`tftpboot 0x00100000 kernel_partition_0.img`<br>
	`tftpboot 0x0FB00000 kernel_partition_1.img`<br>
	`tftpboot 0x1F500000 kernel_partition_2.img`<br>
	`go 0x00100000`




<br>

## Architecture

### Overview
To minimize the code adaptations needed for RODOS when run isolated with multiple partitions, the TSP system in RODOS is not run with a separated abstraction layer for managing the time and space constraints like other TSP Systems would do (see e.g. [ARINC 653](https://en.wikipedia.org/wiki/ARINC_653)). Instead, the partitions themselves coordinate the context switching, which comes with several advantages. The most important ones being less introduced performance overhead when doing a context/partition switch and more simplicity through less abstraction layers.


### Execution Flow
Since there is no master image or a separated abstraction layer for the time and space constraints, all the partitions need to initialize themselves. This is achieved via chain-loading by booting up partition 0 and after initializing the system and the hardware for this image as well as saving the partition context (described in more detail in the next paragraph), we jump to the next partition and do the same. This makes sure that every partition is at the same progress before starting up RODOS main scheduler and running user threads of the partitioned applications.

The last image in the chain-load is the first image to start execution of the scheduler and therefore its user threads. When the time slice of the partition expired, the hardware timer fires and redirects execution to its ISR, which calculates the next partition to execute using a **Round-Robin scheduling algorithm**. It looks up the current partition in the globally generated partition table and automatically determines the RAM address of the next partition in the cycle before initiating the context switch.
The ISR is also used to redirect execution to context switching procedures inside the image. These procedures first save the context of the current partition including the general purpose registers (r0-r14), the program counter (PC), the status registers (CPSR/SPSR), the banked registers for all CPU exception modes (SVC, IRQ, FIQ, ABORT, UNDEFINED), and critical system configuration registers such as the MMU translation table base (TTBR0), the system control register (SCTLR) and the vector base address register (VBAR). Additionally, a Magic Number to mark the context switch is saved in the register r0.

The partition then jumps to the reset label of the partition that should be switched to. One of the first things this new partition does is to check the register r0 for the Magic Number that was set in the old partition. If r0 contains this Magic Number, the partition calls a routine to load the old context of the partition to resume execution.

Although the hardware timer interrupts guarantee hard real-time preemption, the partitions are allowed to switch to another partition even before the time slice of the partition expired. This can be done in the user threads of an application partition via a svc and the address of the partition to switch to as a parameter (These addresses of partitions are automatically deduced from the configuration in the build infrastructure).



### Space Isolation
Space Isolation is guaranteed by the MMU. Each partition maintains its own **isolated Level 1 Page Table** in its own .bss RAM section defined in the generated linker scripts. When a partition is booted, the MMU maps only its own memory boundaries as Read/Write/Execute, the peripherals as well as the shared memory location. No other image/partition can access its memory address space from now on.

When a context switch from Partition A to Partition B happens, the MMU is reconfigured in the following way:

* Before Partition A jumps to Partition B, Partition A must temporarily map Partition B's memory into its own page table. Otherwise, the the jump to the new partition would immediately cause a Prefetch Abort because Partition B's address space is invisible to Partition A. 

* Once Partition B takes over, it is running on its own restored page table. However, to ensure strict isolation, it iterates through the global partition table and explicitly unmaps all other partitions. This removes any residual access rights.


### Inter-Partition Communication
In order to be able to save and load the partition context when switching partitions, a shared memory section in the RAM is needed. For this, the linker script assigns a dedicated 1 KiB slot to each partition based on its image index. The entire CPU state (Registers, PC, CPSR, MMU config) can then be written into it so that it survives the upcoming MMU reconfiguration.






