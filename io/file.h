// fs.h: File System

#pragma once

#include "../disk/disk.h"

#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define MAGIC_NUMBER 0xf0f03410
#define INODES_PER_BLOCK 128
#define POINTERS_PER_INODE 10
#define POINTERS_PER_BLOCK 1024

typedef struct {		// Superblock structure
    uint32_t MagicNumber;	// File system magic number
    uint32_t Blocks;	// Number of blocks in file system
    uint32_t InodeBlocks;	// Number of blocks reserved for inodes
    uint32_t Inodes;	// Number of inodes in file system
} SuperBlock;

typedef struct {
    uint32_t Valid;		// Whether or not inode is valid
    uint32_t Size;		// Size of file
    uint32_t Flag;
    uint32_t children[10];
    int Direct[POINTERS_PER_INODE]; // Direct pointers
    uint32_t Indirect;	// Indirect pointer
} Inode;

typedef union {
    SuperBlock  Super;			    // Superblock
    Inode	    Inodes[INODES_PER_BLOCK];	    // Inode block
    int    Pointers[POINTERS_PER_BLOCK];   // Pointer block
    char	    Data[BLOCK_SIZE];	    // Data block
} Block;

uint32_t    numBlocks;
uint32_t    inodeBlocks;
uint32_t    inodes;
bool *       freeBlocks;

struct Disk        *disk;

void debug(struct Disk *disk);
bool format(struct Disk *disk);

bool mount(struct Disk *disk);

void initialize_inode(Inode node);
bool load_inode(size_t inumber, Inode node);   
bool save_inode(size_t inumber, Inode node);
size_t allocate_free_block();

ssize_t create();
bool    removeFile(size_t inumber);
ssize_t stat(size_t inumber);

ssize_t readFile(size_t inumber, char data, size_t length, size_t offset);
ssize_t writeFile(size_t inumber, char data, size_t length, size_t offset);