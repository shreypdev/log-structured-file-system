#include "file.h"
// Debug file system -----------------------------------------------------------
void debug(struct Disk *disk) {
    Block block;

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
    Block inodeBlock;
    uint32_t numInodes = sizeof(Block)/sizeof(Inode);
    for (uint32_t i=0; i < block.Super.InodeBlocks; i++){
        readDisk(disk, i+1, inodeBlock.Data);
        for (uint32_t j=0; j< numInodes; j++){
            if (inodeBlock.Inodes[j].Valid){
                printf("Inode %d:\n", j+i*numInodes);
                printf("    size: %d bytes\n", inodeBlock.Inodes[j].Size);
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
                    printf("%s\n", directBlockString);;
                }else{
                    printf("    direct blocks:\n");
                }
                if (inodeBlock.Inodes[j].Indirect) {
                    printf("    indirect block: %d\n", inodeBlock.Inodes[j].Indirect);
                    // Load Indirect Block
                    Block indiBlock;
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
                        printf("%s\n", indiString);
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

    Block superBlock;
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
    Block emptyBlock = {0};
    for (uint32_t i = 1; i < superBlock.Super.Blocks; i++){
        writeDisk(disk, i, emptyBlock.Data);
    }

    return true;
}

// Mount file system -----------------------------------------------------------
// bool mount(Disk disk) {
    
//     // if (disk != null){
//     //     return false;
//     // }   
 
    
//     // Read superblock
//     Block superBlock;
//     readDisk(disk, 0, superBlock.Data);
    
//     if (superBlock.Super.MagicNumber != MAGIC_NUMBER){
//         return false;
//     }   

//     if (size(disk) != superBlock.Super.Blocks){
//         return false;
//     }
    
//     if (size(disk)%10 == 0){
//         if (superBlock.Super.InodeBlocks != size(disk)/10){
//             return false;
//         }
//     }else{
//         if (superBlock.Super.InodeBlocks != size(disk)/10+1){
//             return false;
//         }
//     }
    
//     if (superBlock.Super.Inodes != INODES_PER_BLOCK*superBlock.Super.InodeBlocks){
//         return false;
//     }

//     // Set device and mount
//     disk = disk;
//     mountDisk(disk);

//     // Copy metadata
//     this->numBlocks     = superBlock.Super.Blocks;
//     this->inodeBlocks   = superBlock.Super.InodeBlocks;
//     this->inodes        = superBlock.Super.Inodes;
 
//     // Allocate free block bitmap
//     this->freeBlocks = new bool[this->numBlocks];
//     for (uint32_t i = 0; i < this->numBlocks; i++){
//         this->freeBlocks[i] = true;
//     }
//     this->freeBlocks[0] = false;
//     for (uint32_t i = 0; i < this->inodeBlocks; i++){
//         this->freeBlocks[i+1] = false;
//     }

//     Block inodeBlock;
//     for (uint32_t i = 0; i < this->inodeBlocks; i++){
//         //std::cout << "Block num: " << i << "\n";
//         disk->read(i+1, inodeBlock.Data);
//         for (uint32_t j = 0; j < INODES_PER_BLOCK; j++){
//             if (inodeBlock.Inodes[j].Valid){
//                 for (uint32_t k = 0; k < POINTERS_PER_INODE; k++){
//                     if (inodeBlock.Inodes[j].Direct[k]){
//                         this->freeBlocks[inodeBlock.Inodes[j].Direct[k]] = false;
//                     }
//                 }
//                 if (inodeBlock.Inodes[j].Indirect){
//                     this->freeBlocks[inodeBlock.Inodes[j].Indirect] = false;
//                     Block indirectBlock;
//                     disk->read(inodeBlock.Inodes[j].Indirect, indirectBlock.Data);
//                     for (uint32_t k = 0; k < POINTERS_PER_BLOCK; k++){
//                         if (indirectBlock.Pointers[k]){
//                             this->freeBlocks[indirectBlock.Pointers[k]] = false;
//                         }
//                     } 
//                 }
//             }
//         }
//     }
    
//     return true;
// }

