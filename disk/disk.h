#pragma once

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

struct Disk{
   int     FileDescriptor;    // File descriptor of disk image
   size_t  Blocks;	         // Number of blocks in disk image
   size_t  Reads;	            // Number of reads performed
   size_t  Writes;	         // Number of writes performed
   size_t  Mounts;	         // Number of mounts
} ;

// Number of bytes per block
#define BLOCK_SIZE 512

// Default constructor
void intiDisk(struct Disk *disk);

// Destructor
void destructDisk(struct Disk *disk);

// Check parameters
// @param	blocknum    Block to operate on
// @param	data	    Buffer to operate on
// Throws invalid_argument exception on error.
bool sanity_check(struct Disk *disk, int blocknum, char *data);

// Open disk image
// @param	path	    Path to disk image
// @param	nblocks	    Number of blocks in disk image
// Throws runtime_error exception on error.
bool openDisk(struct Disk *disk, const char *path, size_t nblocks);

// Return size of disk (in terms of blocks)
size_t size(struct Disk *disk);

// Return whether or not disk is mounted
bool mountedDisk(struct Disk *disk);

// Increment mounts
void mountDisk(struct Disk *disk);

// Decrement mounts
void unmountDisk(struct Disk *disk);

// Read block from disk
// @param	blocknum    Block to read from
// @param	data	    Buffer to read into
bool readDisk(struct Disk *disk, int blocknum, char *data);

// Write block to disk
// @param	blocknum    Block to write to
// @param	data	    Buffer to write from
bool writeDisk(struct Disk *disk, int blocknum, char *data);