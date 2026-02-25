#!/usr/bin/env python3

"""
This module handles the generation of custom linker scripts for multi-partition
booting. It uses a template linkerscript file and a configuration list given by
config_partition.py to calculate and write specific memory layouts (RAM origin,
shared memory, and boot-chain addresses) for each partition.
"""


import os
from string import Template
import shutil
from config_partition import partition_configs

def generate_from_file(template_path, configs):
    """
    Reads a linker script template and generates individual .ld files for each 
    partition configuration. Cleans the output directory before generation.
    """
    try:
        with open(template_path, "r") as f:
            template_content = f.read()
    except FileNotFoundError:
        print(f"Error: Did not find {template_path}.")
        return

    
    output_dir = "generated_scripts"
    
    # If it already exists, make sure we start from empty directory so that the
    # files are newly generated
    if os.path.exists(output_dir):
        shutil.rmtree(output_dir)

    os.makedirs(output_dir)


    table_lines = []
    for c in configs:
        table_lines.append(f"LONG({c['ram_origin']}); LONG({c['ram_length']});")
    partition_table_content = "\n        ".join(table_lines)

    
    for i, cfg in enumerate(configs):
        t = Template(template_content)
        formatted_content = t.substitute(
            RAM_ORIGIN=cfg["ram_origin"],
            RAM_LENGTH=cfg["ram_length"],
            SHARED_ORIGIN=cfg["shared_origin"],
            SHARED_LENGTH=cfg["shared_length"],
            IMG_INDEX=i,
            IMG_COUNT=len(configs),
            NEXT_IMAGE_ADDRESS=cfg["next_image_address"],
            PARTITION_TABLE_CONTENT=partition_table_content,
            TICKS_VALUE=cfg["partition_switch_interval_ticks"]
        )


        filename = f"linkerscript_partition_{i}_{cfg["ram_origin"]}.ld"
        filepath = os.path.join(output_dir, filename)
        

        with open(filepath, "w") as f:
            f.write(formatted_content)
        
        print(f"Succesfully created: {filepath}")



if __name__ == "__main__":
    generate_from_file(
        "../../../src/bare-metal/raspberrypi3/scripts/"
        "linkerscript_template_multiple_partitions.ld", 
        partition_configs
    )



