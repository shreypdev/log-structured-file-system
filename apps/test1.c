#include "../disk/disk.h"
#include "../io/file.h"

// Macros
#define streq(a, b) (strcmp((a), (b)) == 0)

// Command prototypes
void do_debug(struct Disk *disk, int args, char *arg1, char *arg2);
void do_format(struct Disk *disk, int args, char *arg1, char *arg2);

// Main execution
int main(int argc, char *argv[]) {
    struct Disk	disk;
    intiDisk(&disk);

    if (argc != 3) {
    	fprintf(stderr, "Usage: %s <diskfile> <nblocks>\n", argv[0]);
    	return EXIT_FAILURE;
    }

    if(!openDisk(&disk, argv[1], atoi(argv[2]))){
        fprintf(stderr, "Unable to open disk %s\n", argv[1]);
    	return EXIT_FAILURE;
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

        if (streq(cmd, "debug")) {
            do_debug(&disk, args, arg1, arg2);
        } else if (streq(cmd, "format")) {
            do_format(&disk, args, arg1, arg2);
        }
    }

    return EXIT_SUCCESS;
}

// Command functions

void do_debug(struct Disk *disk, int args, char *arg1, char *arg2) {
    if (args != 1) {
    	printf("Usage: debug\n");
    	return;
    }

    debug(disk);
}

void do_format(struct Disk *disk, int args, char *arg1, char *arg2) {
    if (args != 1) {
    	printf("Usage: format\n");
    	return;
    }

    if (format(disk)) {
    	printf("disk formatted.\n");
    } else {
    	printf("format failed!\n");
    }
}