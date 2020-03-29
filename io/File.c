#include "File.h"

extern int NUM_INODE;
extern int root_inode_index;
extern int magic_number;
extern int mapping_block_start; 	// ( block 9-24 for inode #1 - #2048 )
extern int x;

void initLLFS ( FILE* disk ){

	create_superblock( disk );
	create_free_block_vector( disk );
	create_root ( disk );

	NUM_INODE++;
	update_superblock( disk );
}

void create_sub_directory ( FILE* disk, int parent_dir_node_block_num, char* child_dir_name ){
	
	int free_block_num3;
	read_free_block_vector( disk , &free_block_num3 );
	create_directory_block( disk, free_block_num3 );
	fill_free_block_vector( disk , free_block_num3 );

	int sub_dir_block_num = free_block_num3;    
	int free_block_num4; 
	read_free_block_vector( disk , &free_block_num4 );
	create_directory_inode( disk, free_block_num4, sub_dir_block_num );
	fill_free_block_vector( disk , free_block_num4 );
	
	int next_free_inode_index3;
	find_next_free_inode_num( disk, &next_free_inode_index3 );

	add_mapping( disk, next_free_inode_index3, free_block_num4 );
	
	NUM_INODE++;
	update_superblock( disk );

	int child_dir_inode_num = next_free_inode_index3;   
	edit_parent_dir_block ( disk, parent_dir_node_block_num, child_dir_inode_num, child_dir_name );
}

void make_directory ( FILE* disk, char* input ){
	
	char* parent_directory_name = (char*)calloc(31,1);	
	char* curr_dir_name = (char*)calloc(31,1);	
	char* fake_curr_dir_name = (char*)calloc(31,1);	
	
	const char s[2] = "/";

	fake_curr_dir_name = strtok( input, s );

	strncpy( parent_directory_name, curr_dir_name, 31);
	strncpy( curr_dir_name, fake_curr_dir_name, 31);
	
	fake_curr_dir_name = strtok(NULL, s);
	
	int saved_parent_inode_block_num = -1;
	
		int newest_root_inode_index = get_root_inode_index ( disk );
			
		int root_inode_block_num = find_mapping( disk, newest_root_inode_index );

		short result_block_num4[12];
		read_inode( disk, root_inode_block_num, result_block_num4 );
			
		saved_parent_inode_block_num = root_inode_block_num;
	
	while( fake_curr_dir_name != NULL ) {
		strncpy( parent_directory_name, curr_dir_name, 31);
		strncpy( curr_dir_name, fake_curr_dir_name, 31);
		fake_curr_dir_name = strtok(NULL, s);

		if ( fake_curr_dir_name == NULL ){
			create_sub_directory ( disk, saved_parent_inode_block_num, curr_dir_name );

			printf("\n       $$$$$$$$$$$$$$ we added a sub directory: %s\n\n", curr_dir_name );
		} else {
			int checking_inode_num;		
			short result_block_num[12];
	
			read_inode( disk, saved_parent_inode_block_num, result_block_num );
			
			for ( int i = 0 ; result_block_num[i] != -1 ; i++ ){
				search_file_or_dir( disk, result_block_num[i], curr_dir_name, &checking_inode_num );
				
				if ( checking_inode_num > 0 ){
					saved_parent_inode_block_num = find_mapping( disk, checking_inode_num );
				}
			}
			
			if ( checking_inode_num < 1 ){
				printf( "\n      no such directory in this datablock: '%s'   \n", curr_dir_name );
				break;
			}
		}
	}
	free( parent_directory_name );
	free( curr_dir_name );
	free( fake_curr_dir_name );	
}

int robust_size( FILE* disk, int last_context_size, int saved_parent_inode_block_num ){

	char* buffer = (char*)calloc(32, 1);
    readDisk(disk, saved_parent_inode_block_num, buffer, 32);

	int file_size;
	
	memcpy( &file_size, buffer, 4 );
	
	if ( file_size == last_context_size ){
		
		return 1;	
	}
	return 0;
} 

int robust_write_file( FILE* disk, char* input, int last_context_size ){

	char* parent_directory_name = (char*)calloc(31,1);	
	char* curr_file_name = (char*)calloc(31,1);	
	char* fake_curr_dir_name = (char*)calloc(31,1);	
	
	const char s[2] = "/";

	fake_curr_dir_name = strtok( input, s );		// skip the first command

	strncpy( parent_directory_name, curr_file_name, 31);
	strncpy( curr_file_name, fake_curr_dir_name, 31);
	
	fake_curr_dir_name = strtok(NULL, s);
	
	int saved_parent_inode_block_num = -1;
	int newest_root_inode_index = get_root_inode_index ( disk );	
	int root_inode_block_num = find_mapping( disk, newest_root_inode_index );
	short result_block_num4[12];
	
	read_inode( disk, root_inode_block_num, result_block_num4 );
	saved_parent_inode_block_num = root_inode_block_num;
	
	while( fake_curr_dir_name != NULL ) {

		strncpy( parent_directory_name, curr_file_name, 31);
		strncpy( curr_file_name, fake_curr_dir_name, 31);
		fake_curr_dir_name = strtok(NULL, s);

		if ( fake_curr_dir_name == NULL ){
			
			int checking_inode_num;		
			short result_block_num[12];
	
			read_inode( disk, saved_parent_inode_block_num, result_block_num );
			
			for ( int i = 0 ; result_block_num[i] != -1 ; i++ ){
			
				search_file_or_dir( disk, result_block_num[i], curr_file_name, &checking_inode_num );
				
				if ( checking_inode_num > 0 ){
					
					saved_parent_inode_block_num = find_mapping( disk, checking_inode_num );
				}
			}
			if ( checking_inode_num < 1 ){
				printf( "\n      no such file: '%s'   \n", curr_file_name );
				return 0;
			}
			

			if ( robust_size(disk, last_context_size, saved_parent_inode_block_num) == 1 ){
				return 1;
			}
			
			return 0;
		}
		else{

			int checking_inode_num;		
			short result_block_num[12];
	
			read_inode( disk, saved_parent_inode_block_num, result_block_num );
			
			for ( int i = 0 ; result_block_num[i] != -1 ; i++ ){
			
				search_file_or_dir( disk, result_block_num[i], curr_file_name, &checking_inode_num );
				
				if ( checking_inode_num > 0 ){
					
					saved_parent_inode_block_num = find_mapping( disk, checking_inode_num );
				}
			}
			if ( checking_inode_num < 1 ){
				printf( "\n      no such directory: '%s'   \n", curr_file_name );
				return 0;
			}
		}
	}
	free( parent_directory_name );
	free( curr_file_name );
	free( fake_curr_dir_name );	
	
	return 0;
}

int robust_Rm_dir ( FILE* disk, char* input ){

	char* parent_directory_name = (char*)calloc(31,1);	
	char* curr_file_name = (char*)calloc(31,1);	
	char* fake_curr_dir_name = (char*)calloc(31,1);	
	
	const char s[2] = "/";

	fake_curr_dir_name = strtok( input, s );		// skip the first command

	strncpy( parent_directory_name, curr_file_name, 31);
	strncpy( curr_file_name, fake_curr_dir_name, 31);
	
	fake_curr_dir_name = strtok(NULL, s);
	
	int saved_parent_inode_block_num = -1;
	int newest_root_inode_index = get_root_inode_index ( disk );	
	int root_inode_block_num = find_mapping( disk, newest_root_inode_index );
	short result_block_num4[12];
	
	read_inode( disk, root_inode_block_num, result_block_num4 );
	saved_parent_inode_block_num = root_inode_block_num;
	
	while( fake_curr_dir_name != NULL ) {

		strncpy( parent_directory_name, curr_file_name, 31);
		strncpy( curr_file_name, fake_curr_dir_name, 31);
		fake_curr_dir_name = strtok(NULL, s);

		if ( fake_curr_dir_name == NULL ){
			
			int checking_inode_num;		
			short result_block_num[12];
	
			read_inode( disk, saved_parent_inode_block_num, result_block_num );
			
			for ( int i = 0 ; result_block_num[i] != -1 ; i++ ){
			
				search_file_or_dir( disk, result_block_num[i], curr_file_name, &checking_inode_num );
				
				if ( checking_inode_num > 0 ){
					
					saved_parent_inode_block_num = find_mapping( disk, checking_inode_num );
				}
			}
			if ( checking_inode_num < 1 ){
				//printf( "\n      no such file: '%s'   \n", curr_file_name );
				return 1;
			}
			
			return 0;
		}
		else{

			int checking_inode_num;		
			short result_block_num[12];
	
			read_inode( disk, saved_parent_inode_block_num, result_block_num );
			
			for ( int i = 0 ; result_block_num[i] != -1 ; i++ ){
			
				search_file_or_dir( disk, result_block_num[i], curr_file_name, &checking_inode_num );
				
				if ( checking_inode_num > 0 ){
					
					saved_parent_inode_block_num = find_mapping( disk, checking_inode_num );
				}
			}
			if ( checking_inode_num < 1 ){
				printf( "\n      no such directory: '%s'   \n", curr_file_name );
				return 0;
			}
		}
	}
	free( parent_directory_name );
	free( curr_file_name );
	free( fake_curr_dir_name );	
	
	return 0;
}

int robust_make_directory ( FILE* disk, char* input ){

	char* parent_directory_name = (char*)calloc(31,1);	
	char* curr_file_name = (char*)calloc(31,1);	
	char* fake_curr_dir_name = (char*)calloc(31,1);	
	
	const char s[2] = "/";

	fake_curr_dir_name = strtok( input, s );		// skip the first command

	strncpy( parent_directory_name, curr_file_name, 31);
	strncpy( curr_file_name, fake_curr_dir_name, 31);
	
	fake_curr_dir_name = strtok(NULL, s);
	
	int saved_parent_inode_block_num = -1;
	int newest_root_inode_index = get_root_inode_index ( disk );	
	int root_inode_block_num = find_mapping( disk, newest_root_inode_index );
	short result_block_num4[12];
	
	read_inode( disk, root_inode_block_num, result_block_num4 );
	saved_parent_inode_block_num = root_inode_block_num;
	
	while( fake_curr_dir_name != NULL ) {

		strncpy( parent_directory_name, curr_file_name, 31);
		strncpy( curr_file_name, fake_curr_dir_name, 31);
		fake_curr_dir_name = strtok(NULL, s);

		if ( fake_curr_dir_name == NULL ){
			
			int checking_inode_num;		
			short result_block_num[12];
	
			read_inode( disk, saved_parent_inode_block_num, result_block_num );
			
			for ( int i = 0 ; result_block_num[i] != -1 ; i++ ){
			
				search_file_or_dir( disk, result_block_num[i], curr_file_name, &checking_inode_num );
				
				if ( checking_inode_num > 0 ){
					
					saved_parent_inode_block_num = find_mapping( disk, checking_inode_num );
				}
			}
			if ( checking_inode_num < 1 ){
				printf( "\n      no such file: '%s'   \n", curr_file_name );
				return 0;
			}
			
			return 1;
		}
		else{

			int checking_inode_num;		
			short result_block_num[12];
	
			read_inode( disk, saved_parent_inode_block_num, result_block_num );
			
			for ( int i = 0 ; result_block_num[i] != -1 ; i++ ){
			
				search_file_or_dir( disk, result_block_num[i], curr_file_name, &checking_inode_num );
				
				if ( checking_inode_num > 0 ){
					
					saved_parent_inode_block_num = find_mapping( disk, checking_inode_num );
				}
			}
			if ( checking_inode_num < 1 ){
				printf( "\n      no such directory: '%s'   \n", curr_file_name );
				return 0;
			}
		}
	}
	free( parent_directory_name );
	free( curr_file_name );
	free( fake_curr_dir_name );	
	
	return 0;
}

int comparing_last_compile( FILE* disk, char* input, int last_context_size ){
	
	char tok_string7[10];
	strncpy( tok_string7, input, 6 );
	char* command7 = (char*)calloc(60,1);	
	char s[2] = "/";
	command7 = strtok( tok_string7, s );
	
	if ( strncmp( command7, "Writefile", 6 ) == 0 ){			
		return robust_write_file( disk, input, last_context_size );
	}
	else if ( strncmp( command7, "Rmdir", 5 ) == 0 ){		
		return robust_Rm_dir( disk, input );
	}
	else if ( strncmp( command7, "Rmfile", 6 ) == 0 ){
		return robust_Rm_dir( disk, input );
	}
	else if ( strncmp( command7, "Mkdir", 5 ) == 0 ){		
		return robust_make_directory( disk, input );
	}
	return 1;
}

void robust_check ( FILE* disk ){

	char* buffer = (char*)calloc(BLOCK_SIZE, 1);
    readDisk(disk, 0, buffer, BLOCK_SIZE);

	char* last_input = (char*)calloc(BLOCK_SIZE, 60);
	int last_context_size;
	memcpy( &last_context_size, buffer+16, 4 );
	memcpy( last_input, buffer+20, 60 );
	
	printf("\n (robust check) Last modified command: %s\n\n", last_input);
	
	if ( strncmp ( last_input, "NONE", 4 ) != 0 ){
		int temp = 1;
		temp = comparing_last_compile ( disk, last_input, last_context_size );
		
		if ( temp == 0 ){
			printf("\n   ****something is lost after the last compile****\n");
			printf( " -------------------------------------------------------------------- \n\n" );
		} else {
			printf("\n   ****vdisk is up to date, previous compile successfully****\n");
			printf( " -------------------------------------------------------------------- \n\n" );
		}
	}
	free(buffer);
}

FILE* initDisk ( char* path ){
	FILE* disk;
   if ( fopen(path, "r") == NULL ){
		
		printf( "\n we can't find the old vdisk file, so we will create new one\n");
		printf( " -------------------------------------------------------------------- \n\n" );
		
		disk = fopen(path, "wb+"); // Open the file to be written to in binary mode
	
		char* init = calloc(BLOCK_SIZE*NUM_BLOCKS, 1);
		fwrite(init, BLOCK_SIZE*NUM_BLOCKS, 1, disk);
		free(init);
	}
	else {
		
		disk = fopen(path, "rb+"); // create the file to be written to in binary mode

		robust_check (disk);
	}
	//initLLFS( disk );

	read_superblock ( disk );
	return disk;
}

