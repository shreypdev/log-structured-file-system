#pragma once

#include "../disk/disk.h"

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

void create_superblock( FILE* disk );

void read_superblock( FILE* disk );

void update_superblock( FILE* disk );

void robust_superblock( FILE* disk, char* input, int content_length );

void create_free_block_vector( FILE* disk );

void read_free_block_vector( FILE* disk, int* find_free_block_num );

void fill_free_block_vector( FILE* disk, int fill_block_num );

void delete_free_block_vector( FILE* disk, int del_block_num );

void add_mapping ( FILE* disk, int inode_num, int inode_block_num );

int find_mapping ( FILE* disk, int inode_num );

void delete_mapping ( FILE* disk, int inode_num );

void create_single_indirect ( FILE* disk, short* file_block_num_array, int size_of_array, short* single_indirect_block_num  );

void translate_indirect_to_size ( FILE* disk, short indirect_block_num, short* result_size );

void translate_indirect_to_array ( FILE* disk, short indirect_block_num, short* result_block_num, short array_size );

int get_root_inode_index ( FILE* disk );

void find_next_free_inode_num ( FILE* disk, int* inode_num );

void create_file_inode ( FILE* disk, int free_block_num, short* file_block_num_array, int size_of_array, int size_of_file );

void create_file_single_inode( FILE* disk, int free_block_num, int dir_block_num  );

void create_directory_inode( FILE* disk, int free_block_num, int dir_block_num  );

void read_inode ( FILE* disk, int inode_block_num, short* result_block_num );

void create_directory_block ( FILE* disk, int free_block_num2 );

void extend_parent_dir_block ( FILE* disk, int parent_dir_node_block_num, int* new_block_num );

void edit_parent_dir_block ( FILE* disk, int parent_dir_node_block_num, int child_dir_inode_num, char* child_dir_name );

void del_entry_dir_block ( FILE* disk, int parent_dir_block_num, int del_file_inode_num );

void create_file_block ( FILE* disk, int free_block_num2, char* file_content_buffer );

void search_file_or_dir ( FILE* disk, int dir_block_num, char* file_name, int* finding_inode_num );

void create_root ( FILE* disk );