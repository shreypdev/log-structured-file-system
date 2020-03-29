#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../io/File.c"


int main(int argc, char* argv[]) {
	
FILE* disk;	

	if ( fopen("./disk/vdisk", "r") == NULL ){
		
		printf( "\n we can't find the old vdisk file, so we will create new one\n");
		printf( " -------------------------------------------------------------------- \n\n" );
		
		disk = fopen("./disk/vdisk", "wb+"); // Open the file to be written to in binary mode
	
		char* init = calloc(BLOCK_SIZE*NUM_BLOCKS, 1);
		fwrite(init, BLOCK_SIZE*NUM_BLOCKS, 1, disk);
		free(init);
	}
	else {
		
		disk = fopen("./disk/vdisk", "rb+"); // create the file to be written to in binary mode

		robust_check (disk);
		
		char* init = calloc(BLOCK_SIZE*NUM_BLOCKS, 1);
		fwrite(init, BLOCK_SIZE*NUM_BLOCKS, 1, disk);
		free(init);
	}

	
	initLLFS( disk );

	
	//  robust	: update metadata (inode - size+flag), and save the last command run on the programme, 
	
	//			 if Mkdir/Mkfile/delete involved, compare vdisk content (inode size)

	//			 if we found data not match, we will run the last command again 

	//		     we have to change: FILE* disk = fopen("../disk/vdisk", "wb+")
	
	fclose(disk);
    return 0;
}