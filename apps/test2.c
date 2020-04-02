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
	}
	//initLLFS( disk );

	read_superblock ( disk );
	
	
	
	
	//test case: "Mkdir /home"
	char input1[] = "Mkdir /home";
	command_input ( disk, input1, input1 );
	
	//test case: "Mkdir /home/usr"
	char input2[] = "Mkdir /home/usr";
	command_input ( disk, input2, input2 );

	//test case: "Mkdir /home/usr/shrey"
	char input3[] = "Mkdir /home/usr/shrey";
	command_input ( disk, input3, input3 );
	
	
	//test case: "Writefile /home/usr/shrey/file1.txt" and the content is "hello world I'm Shrey"
	
	char input4[] = "Writefile /home/usr/shrey/file1.txt";
	char file_content[] = "hello world I'm Shrey";
	command_input ( disk, input4, file_content );

	
	
	//test case: "Open /home/usr/shrey/file1.txt"
	char input5[] = "Open /home/usr/shrey/file1.txt";
	command_input ( disk, input5, input5 );

	
	//test case: "Mkdir /home/downloads"
	char input6[] = "Mkdir /home/downloads";
	command_input ( disk, input6, input6 );
	
	
	//test case: "Writefile //home/usr/shrey/testing2.txt" and the content is "hello world hello world"
	
	char input7[] = "Writefile //home/usr/shrey/testing2.txt";
	char file_content_larger2[] = "hello world hello world";
	command_input ( disk, input7, file_content_larger2 );

	

	//test case: file larger than 1 block	
	//test case: "Writefile /home/downloads/testing.txt" and the content is 512*a + 512*b + 2*c
	char input8[] = "Writefile /home/downloads/testing.txt";
	char* file_content_larger = (char*)calloc( 1026, 1 );
	
	for ( int i = 0; i < 150; i++ ){
		file_content_larger[i] = 's';
	}
	for ( int i = 150; i < 300; i++ ){
		file_content_larger[i] = 'h';
	}
	for ( int i = 300; i < 450; i++ ){
		file_content_larger[i] = 'r';
	}
	for ( int i = 450; i < 600; i++ ){
		file_content_larger[i] = 'e';
	}
	for ( int i = 600; i < 750; i++ ){
		file_content_larger[i] = 'y';
	}
	command_input ( disk, input8, file_content_larger );

	
	//test case: "Open /home/downloads/testing.txt"
	char input9[] = "Open /home/downloads/testing.txt";
	command_input ( disk, input9, input9 );
	
	//test case: delete ( Rmfile )
	char input10[] = "Rmfile /home/downloads/testing.txt";
	command_input ( disk, input10, input10 );


	//test case: "list /home/downloads"
	char input11[] = "list /home/downloads";
	command_input ( disk, input11, input11 );
	
	
	//test case: delete ( Rmdir )
	char input12[] = "Rmdir /home/downloads";
	command_input ( disk, input12, input12 );

	
	//test case: "list /home/downloads"
	char input13[] = "list /home/downloads";
	command_input ( disk, input13, input13 );
	
	
	
	//test case: "list /home"
	char input14[] = "list /home";
	command_input ( disk, input14, input14 );	
	
	
	//test case: file create in root dir
	char input15[] = "Writefile /testing3.txt";
	char file_content_larger3[] = "hello world hello world";
	command_input ( disk, input15, file_content_larger3 );
	
	
	//test case: "list /"
	char input16[] = "list /";
	command_input ( disk, input16, input16 );	
	
	
	//test case: "Writefile /home/usr/shrey/testing4.txt" and the content is Empty
	
	char input17[] = "Writefile /home/usr/shrey/testing4.txt";
	char* file_content_empty = NULL;
	command_input ( disk, input17, file_content_empty );
	
	//test case: "list /home/usr/shrey"
	char input18[] = "list /home/usr/shrey";
	command_input ( disk, input18, input18 );	
	
	//test case: "Open /home/usr/shrey/testing4.txt"
	char input19[] = "Open /home/usr/shrey/testing4.txt";
	command_input ( disk, input19, input19 );
	
	
	//test case: 
	char input20[] = "Writefile /home/usr/shrey/test11.txt";
	char* file_content_larger4 = (char*)calloc( 9141, 1 );
	
	for ( int i = 0; i < 5120; i++ ){
		file_content_larger4[i] = 'C'; }
	for ( int i = 5120; i < 9140; i++ ){
		file_content_larger4[i] = 'S'; }
	file_content_larger4[9140] = 'C';
	
	//printf( "\n\n%s\n\n" , file_content_larger4 );
	command_input ( disk, input20, file_content_larger4 );
	
	
	//test case: "Open /home/usr/shrey/test11.txt"
	char input21[] = "Open /home/usr/shrey/test11.txt";
	command_input ( disk, input21, input21 );

	
	printf( "\n        **********END OF TEST CASES***********  \n\n\n" );
	
	
	//  robust	: update metadata (inode - size+flag), and save the last command run on the programme, 
	
	//			 if Mkdir/Mkfile/delete involved, compare vdisk content (inode size)

	//			 if we found data not match, we will run the last command again 

	//		     we have to change: FILE* disk = fopen("../disk/vdisk", "wb+")
	
	fclose(disk);
    return 0;
}