#include "../disk/disk.h"
#include "../io/File_Helper.h"
#include "../io/File.h"

// Macros
#define streq(a, b) (strcmp((a), (b)) == 0)

// Command prototypes
// void do_debug(struct Disk *disk, int args, char *arg1, char *arg2);
// void do_format(struct Disk *disk, int args, char *arg1, char *arg2);
// void do_mount(struct Disk *disk, int args, char *arg1, char *arg2);
// void do_unmount(struct Disk *disk, int args, char *arg1, char *arg2);
// void do_get_free_inode(struct Disk *disk, int args, char *arg1, char *arg2);
void do_mkdir(FILE* disk, int args, char *cmd);

// Main execution
int main(int argc, char *argv[]) {

    if (argc != 2) {
    	fprintf(stderr, "Usage: %s <diskfile> <nblocks>\n", argv[0]);
    	return EXIT_FAILURE;
    }

    FILE* disk = initDisk(argv[1]);

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

        // if (streq(cmd, "debug")) {
        //     do_debug(&disk, args, arg1, arg2);
        // } else if (streq(cmd, "format")) {
        //     do_format(&disk, args, arg1, arg2);
        // } else if (streq(cmd, "mount")) {
        //     do_mount(&disk, args, arg1, arg2);
        // } else if (streq(cmd, "unmount")) {
        //     do_unmount(&disk, args, arg1, arg2);
        // } else if (streq(cmd, "get_free_inode")) {
        //     do_get_free_inode(&disk, args, arg1, arg2);
        // }
        
        if (streq(cmd, "mkdir")) {
            do_mkdir(disk, args, line);
        }
    }

    return EXIT_SUCCESS;
}

// Command functions

// void do_debug(struct Disk *disk, int args, char *arg1, char *arg2) {
//     if (args != 1) {
//     	printf("Usage: debug\n");
//     	return;
//     }

//     debug(disk);
// }

// void do_format(struct Disk *disk, int args, char *arg1, char *arg2) {
//     if (args != 1) {
//     	printf("Usage: format\n");
//     	return;
//     }

//     if (format(disk)) {
//     	printf("disk formatted.\n");
//     } else {
//     	printf("format failed!\n");
//     }
// }

// void do_mount(struct Disk *disk, int args, char *arg1, char *arg2) {
//     if (args != 1) {
//     	printf("Usage: mount\n");
//     	return;
//     }

//     if (mount(disk)) {
//     	printf("disk mounted.\n");
//     } else {
//     	printf("mount failed!\n");
//     }
// }

// void do_unmount(struct Disk *disk, int args, char *arg1, char *arg2) {
//     if (args != 1) {
//     	printf("Usage: unmount\n");
//     	return;
//     }

//     if (unmount(disk)) {
//     	printf("disk unmounted.\n");
//     } else {
//     	printf("unmount failed!\n");
//     }
// }

// void do_get_free_inode(struct Disk *disk, int args, char *arg1, char *arg2) {
//     if (args != 1) {
//     	printf("Usage: get_free_inode\n");
//     	return;
//     }

//     ssize_t inumber = get_free_inode();
//     if (inumber >= 0) {
//     	printf("get_free_inode inode %ld.\n", inumber);
//     } else {
//     	printf("get_free_inode failed!\n");
//     }
// }

void do_mkdir(FILE* disk, int args, char *cmd) {
    if (args != 2) {
    	printf("Usage: mkdir <path>\n");
    	return;
    }

    make_directory(disk, cmd);
}