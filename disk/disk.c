const int BLOCK_SIZE = 512;

void writeBlock(FILE* disk, int blockNum, char* data, int size){
    fseek(disk, blockNum * BLOCK_SIZE, SEEK_SET);
    fwrite(data, size, 1, disk); 
}

void readBlock(FILE* disk, int blockNum, char* buffer, int size){
    fseek(disk, blockNum * BLOCK_SIZE, SEEK_SET);
    fread(buffer, size, 1, disk);
}