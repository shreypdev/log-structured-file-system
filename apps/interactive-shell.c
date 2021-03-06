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

   FILE* disk;	

	char* diskPath = "./disk/vdisk";

	if ( fopen(diskPath, "r") == NULL ){
		
		printf( "\n we can't find the old vdisk file, so we will create new one\n");
		printf( " -------------------------------------------------------------------- \n\n" );
		
		disk = fopen(diskPath, "wb+"); // Open the file to be written to in binary mode
	
		char* init = calloc(BLOCK_SIZE*NUM_BLOCKS, 1);
		fwrite(init, BLOCK_SIZE*NUM_BLOCKS, 1, disk);
		free(init);
		initLLFS( disk );
		fclose(disk);
		disk = fopen(diskPath, "rb+"); // create the file to be written to in binary mode
		robust_check (disk);
		read_superblock ( disk );
	} else {
		
		disk = fopen(diskPath, "rb+"); // create the file to be written to in binary mode
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

		strcpy(line, strtok(line, "\n"));

        if (streq(cmd, "Mkdir")) {
			if (args != 2) {
				printf("Usage: Mkdir <path>\n");
				continue;
			}

			command_input ( disk, line, line );
        } else if (streq(cmd, "Writefile")) {
			if (args != 2) {
				printf("Usage: Writefile <path>\n");
				continue;
			}

			char content[BUFSIZ];
			printf("Enter content: ");
    		fgets(content, BUFSIZ, stdin);
			
			command_input ( disk, line, content );
        } else if (streq(cmd, "Open")) {
			if (args != 2) {
				printf("Usage: Open <path>\n");
				continue;
			}
			
			command_input ( disk, line, line );
        } else if (streq(cmd, "Rmfile")) {
			if (args != 2) {
				printf("Usage: Rmfile <path>\n");
				continue;
			}
			
			command_input ( disk, line, line );
        } else if (streq(cmd, "Rmdir")) {
			if (args != 2) {
				printf("Usage: Rmdir <path>\n");
				continue;
			}
			
			command_input ( disk, line, line );
        } else if (streq(cmd, "list")) {
			if (args != 2) {
				printf("Usage: list <path>\n");
				continue;
			}
			
			command_input ( disk, line, line );
        }
    }

    return EXIT_SUCCESS;
}