#pragma once


#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    uint32_t start_addr;
    uint32_t length;
} PartitionEntry;


extern PartitionEntry __partition_table_start__[];
extern uint32_t       __image_count__;

#ifdef __cplusplus
}
#endif


/***********************************************************************
 * RAM addresses of used Partitions
 **********************************************************************/
#define PARTITION_0_ADDRESS (__partition_table_start__[0].start_addr)
#define PARTITION_1_ADDRESS (__partition_table_start__[1].start_addr)
#define PARTITION_2_ADDRESS (__partition_table_start__[2].start_addr)

#define PARTITION_LAST_ADDRESS (__partition_table_start__[((uint32_t)&__image_count__) - 1].start_addr)
