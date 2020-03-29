#pragma once

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// Number of bytes per block
#define BLOCK_SIZE 512

// Read block from disk
// @param   disk        Disk File
// @param	blocknum    Block to read from
// @param	data	      Buffer to read into
// @param   size        Current size of file
void readDisk(FILE* disk, int blockNum, char* data, int size);

// Write block to disk
// @param   disk        Disk File
// @param	blocknum    Block to read from
// @param	data	      Buffer to read into
// @param   size        Current size of file
void writeDisk(FILE* disk, int blockNum, char* buffer, int size);