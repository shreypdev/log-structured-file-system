#pragma once

#include "../disk/disk.h"
#include "File_Helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define NUM_BLOCKS 4096

void initLLFS ( FILE* disk );

void create_sub_directory ( FILE* disk, int parent_dir_node_block_num, char* child_dir_name );

void make_directory ( FILE* disk, char* input );

void robust_check ( FILE* disk );

FILE* initDisk ( char* path );