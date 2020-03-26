#include "disk.h"

void intiDisk(struct Disk *disk) {
   disk->FileDescriptor = 0;
   disk->Blocks = 9;
   disk->Reads = 0;
   disk->Writes = 0;
   disk->Mounts = 0;
   return;
}

void destructDisk(struct Disk *disk) {
    if (disk->FileDescriptor > 0) {
    	printf("%lu disk block reads\n", disk->Reads);
    	printf("%lu disk block writes\n", disk->Writes);
    	close(disk->FileDescriptor);
    	disk->FileDescriptor = 0;
    }
}

bool sanity_check(struct Disk *disk, int blocknum, char *data) {
   char what[BUFSIZ];

    if (blocknum < 0) {
        snprintf(what, BUFSIZ, "blocknum (%d) is negative!", blocknum);
        return false;
    }

    if (blocknum >= (int)disk->Blocks) {
        snprintf(what, BUFSIZ, "blocknum (%d) is too big!", blocknum);
        return false;
    }

    if (data == NULL) {
        snprintf(what, BUFSIZ, "null data pointer!");
        return false;
    }

   return true;
}

bool openDisk(struct Disk *disk, const char *path, size_t nblocks) {
   disk->FileDescriptor = open(path, O_RDWR|O_CREAT, 0600);
    if (disk->FileDescriptor < 0) {
        char what[BUFSIZ];
    	snprintf(what, BUFSIZ, "Unable to open %s: %s", path, strerror(errno));
    	return false;
    }

    if (ftruncate(disk->FileDescriptor, nblocks*BLOCK_SIZE) < 0) {
    	char what[BUFSIZ];
    	snprintf(what, BUFSIZ, "Unable to open %s: %s", path, strerror(errno));
    	return false;
    }

    disk->Blocks = nblocks;
    disk->Reads  = 0;
    disk->Writes = 0;
    return true;
}

size_t size(struct Disk *disk) { return disk->Blocks; }

bool mountedDisk(struct Disk *disk) { return disk->Mounts > 0; }

void mountDisk(struct Disk *disk) { disk->Mounts++; }

void unmountDisk(struct Disk *disk) { if (disk->Mounts > 0) disk->Mounts--; }

bool readDisk(struct Disk *disk, int blocknum, char *data) {
    if (!sanity_check(disk, blocknum, data)){ return false; }

    if (lseek(disk->FileDescriptor, blocknum*BLOCK_SIZE, SEEK_SET) < 0) {
        char what[BUFSIZ];
        snprintf(what, BUFSIZ, "Unable to lseek %d: %s", blocknum, strerror(errno));
        return false;
    }

    if (read(disk->FileDescriptor, data, BLOCK_SIZE) != BLOCK_SIZE) {
        char what[BUFSIZ];
        snprintf(what, BUFSIZ, "Unable to read %d: %s", blocknum, strerror(errno));
        return false;
    }

    disk->Reads++;
    return true;
}

bool writeDisk(struct Disk *disk, int blocknum, char *data) {
    if(!sanity_check(disk, blocknum, data)){ return false; }

    if (lseek(disk->FileDescriptor, blocknum*BLOCK_SIZE, SEEK_SET) < 0) {
    	char what[BUFSIZ];
    	snprintf(what, BUFSIZ, "Unable to lseek %d: %s", blocknum, strerror(errno));
    	return false;
    }

    if (write(disk->FileDescriptor, data, BLOCK_SIZE) != BLOCK_SIZE) {
    	char what[BUFSIZ];
    	snprintf(what, BUFSIZ, "Unable to write %d: %s", blocknum, strerror(errno));
    	return false;
    }

    disk->Writes++;
    return true;
}