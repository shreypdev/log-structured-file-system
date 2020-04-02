#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "../io/File.c"

// Macros
#define streq(a, b) (strcmp((a), (b)) == 0)

// Main execution
int main(int argc, char *argv[]) {

    if (argc != 2) {
    	fprintf(stderr, "Usage: %s <diskfile>\n", argv[0]);
    	return EXIT_FAILURE;
    }

    FILE* disk;	

	if ( fopen("./disk/vdisk", "r") == NULL ){
		
		printf( "\n we can't find the old vdisk file, so we will create new one\n");
		printf( " -------------------------------------------------------------------- \n\n" );
		
		disk = fopen("./disk/vdisk", "wb+"); // Open the file to be written to in binary mode
	
		char* init = calloc(BLOCK_SIZE*NUM_BLOCKS, 1);
		fwrite(init, BLOCK_SIZE*NUM_BLOCKS, 1, disk);
		free(init);
		initLLFS( disk );
		fclose(disk);
		disk = fopen("./disk/vdisk", "rb+"); // create the file to be written to in binary mode
		robust_check (disk);
		read_superblock ( disk );
	} else {
		
		disk = fopen("./disk/vdisk", "rb+"); // create the file to be written to in binary mode
		robust_check (disk);
		read_superblock ( disk );
	}

    while (true) {
	    char line[BUFSIZ], cmd[BUFSIZ], arg1[BUFSIZ], arg2[BUFSIZ];

    	fprintf(stderr, "sfs> ");
    	fflush(stderr);

    	if (fgets(line, BUFSIZ, stdin) == NULL) {
    	    break;
    	}

    	int args = sscanf(line, "%s %s %s", cmd, arg1, arg2);
    	if (args == 0) {
    	    continue;
	    }

        if (streq(cmd, "Mkdir")) {
			if (args != 2) {
				printf("Usage: Mkdir <path>\n");
				return EXIT_FAILURE;
			}
			strcpy(line, strtok(line, "\n"));
			
			command_input ( disk, line, line );
        }
    }

    return EXIT_SUCCESS;
}