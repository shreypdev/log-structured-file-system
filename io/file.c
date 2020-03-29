#include "file.h"

// Debug file system -----------------------------------------------------------
void debug(struct Disk *disk) {
    union Block block;

    // Read Superblock
    readDisk(disk, 0, block.Data);
    
    printf("SuperBlock:\n");
    if (block.Super.MagicNumber == MAGIC_NUMBER) {
        printf("    magic number is valid\n");
    }
    printf("    %u blocks\n"         , block.Super.Blocks);
    printf("    %u inode blocks\n"   , block.Super.InodeBlocks);
    printf("    %u inodes\n"         , block.Super.Inodes);

    // Read Inode blocks
    union Block inodeBlock;
    uint32_t numInodes = sizeof(union Block)/sizeof(struct Inode);
    for (uint32_t i=0; i < block.Super.InodeBlocks; i++){
        readDisk(disk, i+1, inodeBlock.Data);
        for (uint32_t j=0; j< numInodes; j++){
            if (inodeBlock.Inodes[j].Valid){
                printf("Inode %d:\n", j+i*numInodes);
                printf("    flag: %d\n", inodeBlock.Inodes[j].Flag);
                printf("    size: %d bytes\n", inodeBlock.Inodes[j].Size);
                printf("    name: %s\n", inodeBlock.Inodes[j].Name);
                //uint32_t directCounter = 0;
                char *directBlockString = "    direct blocks:";
                bool directFlag = false;
                for (uint32_t k=0; k < POINTERS_PER_INODE; k++){
                    if (inodeBlock.Inodes[j].Direct[k]){
                        directFlag = true;
                        strcat(directBlockString, " ");
                        // strcat(directBlockString, itoa(inodeBlock.Inodes[j].Direct[k]));
                    }
                }
                if (directFlag){
                    printf("%s\n", directBlockString);
                }else{
                    printf("    direct blocks:\n");
                }
                if (inodeBlock.Inodes[j].Indirect) {
                    printf("    indirect block: %d\n", inodeBlock.Inodes[j].Indirect);
                    // Load Indirect Block
                    union Block indiBlock;
                    readDisk(disk, inodeBlock.Inodes[j].Indirect, indiBlock.Data);
                    char *indiString = "    indirect data blocks:";
                    bool indiFlag = false;
                    for (uint32_t k = 0; k < POINTERS_PER_BLOCK; k++){
                        if (indiBlock.Pointers[k]) {
                            indiFlag = true;
                            strcat(indiString, " ");
                            // strcat(indiString, itoa(indiBlock.Pointers[k]));
                        }
                    }
                    if (indiFlag) {
                        printf("%d\n", indiFlag);
                    }
                }
                
            }
        }
    }
}

// Format file system ----------------------------------------------------------
bool format(struct Disk *disk) {
    // Write superblock
    if (mountedDisk(disk)){
        return false;
    }

    union Block superBlock;
    superBlock.Super.MagicNumber    = MAGIC_NUMBER;
    superBlock.Super.Blocks         = size(disk);
    if (size(disk)%10 == 0){
        superBlock.Super.InodeBlocks    = size(disk)/10;
    }else{
        superBlock.Super.InodeBlocks    = size(disk)/10+1;
    }
    superBlock.Super.Inodes = superBlock.Super.InodeBlocks*INODES_PER_BLOCK;
    int superBlockLocation = 0;
    writeDisk(disk, superBlockLocation, superBlock.Data);
    
    // Clear all other blocks
    union Block emptyBlock;
    for (uint32_t i = 1; i < superBlock.Super.Blocks; i++){
        writeDisk(disk, i, emptyBlock.Data);
    }

    return true;
}

// Mount file system -----------------------------------------------------------
bool mount(struct Disk *disk) {

    if (fileSystemDisk){
        return false;
    }   
 
    
    // Read superblock
    union Block superBlock;
    readDisk(disk, 0, superBlock.Data);
    
    if (superBlock.Super.MagicNumber != MAGIC_NUMBER){
        return false;
    }   

    if (size(disk) != superBlock.Super.Blocks){
        return false;
    }
    
    if (size(disk)%10 == 0){
        if (superBlock.Super.InodeBlocks != size(disk)/10){
            return false;
        }
    }else{
        if (superBlock.Super.InodeBlocks != size(disk)/10+1){
            return false;
        }
    }
    
    if (superBlock.Super.Inodes != INODES_PER_BLOCK*superBlock.Super.InodeBlocks){
        return false;
    }

    // Set device and mount
    fileSystemDisk = disk;
    mountDisk(disk);

    // Copy metadata
    numBlocks     = superBlock.Super.Blocks;
    inodeBlocks   = superBlock.Super.InodeBlocks;
    inodes        = superBlock.Super.Inodes;
 
    // Allocate free block bitmap
    freeBlocks = (bool *)malloc(numBlocks);
    for (uint32_t i = 0; i < numBlocks; i++){
        freeBlocks[i] = true;
    }
    freeBlocks[0] = false;
    for (uint32_t i = 0; i < inodeBlocks; i++){
        freeBlocks[i+1] = false;
    }

    union Block inodeBlock;
    for (uint32_t i = 0; i < inodeBlocks; i++){
        //std::cout << "Block num: " << i << "\n";
        readDisk(disk, i+1, inodeBlock.Data);
        for (uint32_t j = 0; j < INODES_PER_BLOCK; j++){
            if (inodeBlock.Inodes[j].Valid){
                for (uint32_t k = 0; k < POINTERS_PER_INODE; k++){
                    if (inodeBlock.Inodes[j].Direct[k]){
                        freeBlocks[inodeBlock.Inodes[j].Direct[k]] = false;
                    }
                }
                if (inodeBlock.Inodes[j].Indirect){
                    freeBlocks[inodeBlock.Inodes[j].Indirect] = false;
                    union Block indirectBlock;
                    readDisk(disk, inodeBlock.Inodes[j].Indirect, indirectBlock.Data);
                    for (uint32_t k = 0; k < POINTERS_PER_BLOCK; k++){
                        if (indirectBlock.Pointers[k]){
                            freeBlocks[indirectBlock.Pointers[k]] = false;
                        }
                    } 
                }
            }
        }
    }
    
    return true;
}

// Unmount file system -----------------------------------------------------------
bool unmount(struct Disk *disk) {

    if (!fileSystemDisk){
        return false;
    }   

    // Set device and mount
    fileSystemDisk = NULL;
    unmountDisk(disk);
    return true;
}

void initialize_inode(struct Inode *node) {
    for (uint32_t i = 0; i < POINTERS_PER_INODE; i++) {
        node->Direct[i] = 0;
    }
    node->Indirect  = 0;
    node->Size      = 0;
}

ssize_t get_free_inode() {
    // Locate free inode in inode table
    ssize_t inodeNumber = -1;
    for (uint32_t i = 0; i < inodeBlocks; i++) {
        union Block inodeBlock;
        readDisk(fileSystemDisk, i+1, inodeBlock.Data);
        for (uint32_t j = 0; j < INODES_PER_BLOCK; j++){
            if (!inodeBlock.Inodes[j].Valid){
                inodeBlock.Inodes[j].Valid = 1;
                initialize_inode(&inodeBlock.Inodes[j]);
                writeDisk(fileSystemDisk, i+1, inodeBlock.Data);
                printf("I am registered in disk");
                inodeNumber = j+INODES_PER_BLOCK*i;
                break;
            }
        }
        if (inodeNumber != -1) {
            break;
        }
    }
    // Record inode if found   
    return inodeNumber;
}

ssize_t validatePathAndGetLastInodeID(char **pathAsArray, int pathAsArrayLength) {
    ssize_t currentWorkingInodeID = 0;
    for(int i=1; i<pathAsArrayLength-1; i++){
        struct Inode loadedInode;
        if(!load_inode(currentWorkingInodeID, &loadedInode)){
            return -1;
        }
        for(int j=0; j<sizeof(loadedInode.ChildList) / sizeof(loadedInode.ChildList[0]); j++){
            ssize_t currentChildInodeID = (ssize_t) loadedInode.ChildList[j];
            struct Inode loadedCurrentChildInode;
            if(!load_inode(currentChildInodeID, &loadedCurrentChildInode)){
                return -1;
            }
            printf("%ld", currentChildInodeID);
            printf("%s", pathAsArray[i]);
            printf("%s, ", loadedCurrentChildInode.Name);
            if(strcmp(loadedCurrentChildInode.Name, pathAsArray[i]) == 0){
                currentWorkingInodeID = currentChildInodeID;
                break;
            }
        }
    }
    return currentWorkingInodeID;
}

bool makeDir(char *path) {

    //  splitting path in an array
    char *pathAsArray[CHILD_AND_DEPTH];
    int length = 0;

    char *p = strtok (path, "/");
    while (p != NULL)
    {
        pathAsArray[length++] = p;
        p = strtok (NULL, "/");
    }

    //  validate the given path and requesting InodeID of the last dir in which we will create new dir
    ssize_t lastInodeID = validatePathAndGetLastInodeID(pathAsArray, length);
    if(lastInodeID < 0){
        printf("Invallid Path");
        return false;
    }

    //  get next free inode
    ssize_t inodeID = get_free_inode();
    if(inodeID < 0){
        printf("File System Error: invalid inode recieved");
        return false;
    }

    //  load new inode to add metadata and then save
    struct Inode loadedInode;
    if(!load_inode(inodeID, &loadedInode)){
        printf("File System Error: not able to load newly created inode");
        return false;
    }

    loadedInode.Flag = 0;
    loadedInode.Name = pathAsArray[length-1];
    loadedInode.Size = 0;

    if(!save_inode(inodeID, &loadedInode)){
        printf("File System Error: not able to save newly created inode");
        return false;
    }

    //  update metadata "children" of parent dir
    if(inodeID > 0){
        
        struct Inode loadedLastInode;
        if(!load_inode(lastInodeID, &loadedLastInode)){
            printf("File System Error: not able to load parent inode");
            return false;
        }
        int ChildListLen = -1;
        while (loadedLastInode.ChildList[++ChildListLen] != 0) { /* do nothing */}
        loadedLastInode.ChildList[ChildListLen] = (uint32_t)inodeID;

        if(!save_inode(lastInodeID, &loadedLastInode)){
            printf("File System Error: not able to save parent inode");
            return false;
        }
    }
    
    return true;
}

bool load_inode(size_t inumber, struct Inode *node) {
    union Block nodeBlock;
    readDisk(fileSystemDisk, inumber/INODES_PER_BLOCK+1, nodeBlock.Data);
    *node = nodeBlock.Inodes[inumber%INODES_PER_BLOCK];
    if (node->Valid) {
        return true;
    }
    return false;
} 

bool save_inode(size_t inumber, struct Inode *node){
 
    union Block nodeBlock;
    readDisk(fileSystemDisk, inumber/INODES_PER_BLOCK+1, nodeBlock.Data);
    nodeBlock.Inodes[inumber%INODES_PER_BLOCK] = *node;

    // union Block indirectBlock;
    // readDisk(fileSystemDisk, node->Indirect, indirectBlock.Data);
    
    writeDisk(fileSystemDisk, inumber/INODES_PER_BLOCK+1, nodeBlock.Data);
    
    if (node->Valid) {
        return true;
    }
    return false;
}