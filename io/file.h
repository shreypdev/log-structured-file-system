#pragma once

#include "../disk/disk.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define MAGIC_NUMBER 0xf0f03410
#define INODES_PER_BLOCK 128
#define POINTERS_PER_INODE 10
#define POINTERS_PER_BLOCK 1024
#define CHILD_AND_DEPTH 16

struct SuperBlock // Superblock structure
{
    uint32_t MagicNumber; // File system magic number
    uint32_t Blocks; // Number of blocks in file system
    uint32_t InodeBlocks; // Number of blocks reserved for inodes
    uint32_t Inodes; // Number of inodes in file system
};

struct Inode
{
    uint32_t Valid; // Whether or not inode is valid
    char *Name;
    uint32_t Size; // Size of file
    uint32_t Flag; //0 for files, 1 for directory
    uint32_t ChildList[CHILD_AND_DEPTH]; //list of child inodes
    int Direct[POINTERS_PER_INODE]; // Direct pointers
    uint32_t Indirect; // Indirect pointer
};

union Block
{
    struct SuperBlock  Super;
    struct Inode       Inodes[INODES_PER_BLOCK];
    int                Pointers[POINTERS_PER_BLOCK];
    char               Data[BLOCK_SIZE];
};

// uint32_t    numblocks;
// uint32_t    inodeblocks;
// uint32_t    inodes;
// bool*       freeblocks;

uint32_t numBlocks;
uint32_t inodeBlocks;
uint32_t inodes;
bool* freeBlocks;

struct Disk *fileSystemDisk; // = {0, 0, 0, 0, 0};

void debug(struct Disk *disk);
bool format(struct Disk *disk);

bool mount(struct Disk *disk);
bool unmount(struct Disk *disk);

void initialize_inode(struct Inode *node);
ssize_t get_free_inode();

ssize_t validatePathAndGetLastInodeID(char **pathAsArray, int pathAsArrayLength);
bool makeDir(char *path);

bool load_inode(size_t inumber, struct Inode *node);   
bool save_inode(size_t inumber, struct Inode *node);
size_t allocate_free_block();


bool    removeFile(size_t inumber);
ssize_t stat(size_t inumber);

ssize_t readFile(size_t inumber, char data, size_t length, size_t offset);
ssize_t writeFile(size_t inumber, char data, size_t length, size_t offset);