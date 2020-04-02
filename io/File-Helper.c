const int NUM_BLOCKS = 4096;
const int INODE_SIZE = 32;

int NUM_INODE = 0;
int root_inode_index = -1;
int magic_number = 1;
int mapping_block_start = 9; 	// ( block 9-24 for inode #1 - #2048 )
int x=1;


void create_superblock( FILE* disk ){
	
	char* buffer = (char*)calloc(BLOCK_SIZE, 1);
	
	memcpy( buffer, &magic_number, 4 );
	//magic_number++;
	memcpy( buffer+4, &NUM_BLOCKS, 4 );
	memcpy( buffer+8, &NUM_INODE, 4 );
	memcpy( buffer+12, &root_inode_index, 4 );
	memcpy( buffer+20, "NONE", 60 );
	
	writeBlock(disk, 0, buffer, BLOCK_SIZE);
	free(buffer);
}


void read_superblock( FILE* disk ){				// block 0
	
	char* buffer = (char*)calloc(BLOCK_SIZE, 1);
    readBlock(disk, 0, buffer, BLOCK_SIZE);	

	memcpy( &NUM_INODE, buffer+8, 4 );
	memcpy( &root_inode_index, buffer+12, 4 );

	/*
	int temp_NUM_inode;	
	memcpy( &temp_NUM_inode, buffer+8, 4 );
	printf("testing num of inode we have: %d\n", temp_NUM_inode);	
	
	int temp_root_inode_index;
	memcpy( &temp_root_inode_index, buffer+12, 4 );
	printf("testing temp_root_inode_index: %d\n", temp_root_inode_index);	
	*/
	free(buffer);
}


int get_root_inode_index( FILE* disk ){
	
	char* buffer = (char*)calloc(BLOCK_SIZE, 1);
    readBlock(disk, 0, buffer, BLOCK_SIZE);	
	
	int temp_root_inode_index;
	memcpy( &temp_root_inode_index, buffer+12, 4 );
	//printf("testing temp_root_inode_index: %d\n", temp_root_inode_index);	
	
	free(buffer);
	
	return temp_root_inode_index;
}


void update_superblock( FILE* disk ){

	char* buffer = (char*)calloc(BLOCK_SIZE, 1);
	
    readBlock(disk, 0, buffer, BLOCK_SIZE);
	
	memcpy( buffer+4, &NUM_BLOCKS, 4 );
	memcpy( buffer+8, &NUM_INODE, 4 );
	memcpy( buffer+12, &root_inode_index, 4 );
	
	writeBlock(disk, 0, buffer, BLOCK_SIZE);
	
	free(buffer);
}


void robust_superblock( FILE* disk, char* input, int content_length ){

	char* buffer = (char*)calloc(BLOCK_SIZE, 1);
    readBlock(disk, 0, buffer, BLOCK_SIZE);

	//int length = strlen(input);
	//printf("\nrobust: %s\n", input);
	
	memcpy( buffer+16, &content_length, 4 );
	memcpy( buffer+20, input, 60 );
	
	writeBlock(disk, 0, buffer, BLOCK_SIZE);	
	free(buffer);
}


void create_free_block_vector( FILE* disk ){      // block 1 - 8

	for ( int j = 1 ; j < 9 ; j++ ) {
	
		if ( j == 1 ){
			
			char free_block_content[BLOCK_SIZE];

			for ( int i = 0 ; i < BLOCK_SIZE; i++ ){
				if ( i < 25 ){							// reserved block 0 - 24
					free_block_content[i] = '0';
				} else {
					free_block_content[i] = '1';
				}
			}
			writeBlock(disk, 1, free_block_content, BLOCK_SIZE);
			
		} else {
	
			char free_block_content2[BLOCK_SIZE];
			
			for ( int i = 0 ; i < BLOCK_SIZE; i++ ){
				free_block_content2[i] = '1';
			}

			writeBlock(disk, j, free_block_content2, BLOCK_SIZE);
		}
	}
}


void read_free_block_vector( FILE* disk, int* find_free_block_num ){      // block 1 - 8
	
	int find_signal = 0;
	int free_block_index = 0;
	
	for ( int i = 1; (i < 9)&&(find_signal == 0) ; i++ ){	
		
		char* buffer5 = (char*)calloc(BLOCK_SIZE, 1);

		readBlock(disk, i, buffer5, BLOCK_SIZE);
		
		for ( int j=0 ; (j < BLOCK_SIZE)&&(find_signal == 0) ; j++ ){
			
			if ( buffer5[j] == '1' ){

				find_signal = 1;
				free_block_index = j + ((i-1)*512);
			}			
		}
		free(buffer5);	
	}
	memcpy( find_free_block_num, &free_block_index, 4 );
}


void fill_free_block_vector( FILE* disk, int fill_block_num ){      // block 1 - 8
	
	int free_block_num = fill_block_num / 128; 
	int remainder = fill_block_num % 128;
	char* buffer5 = (char*)calloc(BLOCK_SIZE, 1);	

	readBlock(disk, free_block_num+1, buffer5, BLOCK_SIZE);
	buffer5[remainder] = '0';
	writeBlock(disk, free_block_num+1, buffer5, BLOCK_SIZE);
	
	free(buffer5);		
}


void delete_free_block_vector( FILE* disk, int del_block_num ){      // block 1 - 8
	
	int free_block_num = del_block_num / 128; 
	int remainder = del_block_num % 128;
	char* buffer5 = (char*)calloc(BLOCK_SIZE, 1);	

	readBlock(disk, free_block_num+1, buffer5, BLOCK_SIZE);
	buffer5[remainder] = '1';
	writeBlock(disk, free_block_num+1, buffer5, BLOCK_SIZE);
	
	free(buffer5);	
}


void add_mapping ( FILE* disk, int inode_num, int inode_block_num ){	// assume max inode=2048, 2048*4/512=16

	int temp_num = (inode_num-1) / 128; 								// inode#-1 because #1 at position#0
	int mapping_block = mapping_block_start + temp_num;    				// ( block 9 - 24 )
	char* buffer6 = (char*)calloc(BLOCK_SIZE, 1);

	readBlock(disk, mapping_block, buffer6, BLOCK_SIZE);

	memcpy( buffer6 + ((inode_num-1) * 4), &inode_block_num, 4 ); 
	
	writeBlock(disk, mapping_block, buffer6, BLOCK_SIZE );

	free(buffer6);	
}


void find_next_free_inode_num ( FILE* disk, int* inode_num ){
	
	int find_signal = 0;
	int free_inode_index = 0;

	for ( int i = 9; (i < 24)&&(find_signal == 0) ; i++ ){	
		
		char* buffer5 = (char*)calloc(BLOCK_SIZE, 1);
		
		readBlock(disk, i, buffer5, BLOCK_SIZE);
		
		for ( int j=0 ; (j < 128 )&&(find_signal == 0) ; j++ ){
			
			int temp_inode_block_num;
			
			memcpy( &temp_inode_block_num, (buffer5 + (j*4)) , 4 ); 
			
			if ( temp_inode_block_num == 0 ){

				find_signal = 1;
				free_inode_index = j + ((i-9)*128) + 1;		// adding 1 to let #0 position for inode#1
			}			
		}
		free(buffer5);	
	}
	memcpy( inode_num, &free_inode_index, 4 );
}


int find_mapping ( FILE* disk, int inode_num ){

	int temp_num = (inode_num-1) / 128; 
	int mapping_block = mapping_block_start + temp_num;    // ( block 9 - 24 )

	char* buffer6 = (char*)calloc(BLOCK_SIZE, 1);
	readBlock(disk, mapping_block, buffer6, BLOCK_SIZE);
	
	int temp_block_num;
	memcpy( &temp_block_num, buffer6 + ((inode_num-1) * 4), 4 ); 

	free(buffer6);	
	return temp_block_num;
}




void delete_mapping ( FILE* disk, int inode_num ){		// assume max inode = 2048

	int temp_num = (inode_num-1) / 128; 
	int mapping_block = mapping_block_start + temp_num;    // ( block 9 - 24 )

	char* buffer6 = (char*)calloc(BLOCK_SIZE, 1);
	readBlock(disk, mapping_block, buffer6, BLOCK_SIZE);
	
	int clear_inode_block_num = 0;
	memcpy( buffer6 + ((inode_num-1) * 4), &clear_inode_block_num, 4 ); 

	//printf("....................clear_inode_block_num.....................: %d\n", inode_num);

	writeBlock(disk, mapping_block, buffer6, BLOCK_SIZE );

	free(buffer6);	
}



void create_single_indirect ( FILE* disk, short* file_block_num_array, int size_of_array, short* single_indirect_block_num  ){
	

	int free_block_num4;									// block for file content
	read_free_block_vector( disk , &free_block_num4 );
	fill_free_block_vector( disk , free_block_num4 );	
	
	char* single_indir_content = (char*)calloc(BLOCK_SIZE, 1);
	
	for ( int i = 10 ; i < size_of_array ; i++){
		memcpy( single_indir_content+((i-10)*2), &file_block_num_array[i] , 2 );
	}

	writeBlock(disk, free_block_num4, single_indir_content, BLOCK_SIZE);

	short temp	= (short) free_block_num4; 
	
	memcpy( single_indirect_block_num, &temp, 2);
}



void translate_indirect_to_size ( FILE* disk, short indirect_block_num, short* result_size ){		

	char* buffer = (char*)calloc(BLOCK_SIZE, 1);
    readBlock(disk, indirect_block_num, buffer, BLOCK_SIZE);
	
	short temp_size = 0;
	short temp_block_num = 1;
	
	for ( int i = 0 ; i < (BLOCK_SIZE/2) && (temp_block_num!= 0); i++ ){

		memcpy( &temp_block_num, buffer + (i*2), 2 );
		
		if ( temp_block_num > 0 ){
			temp_size++;
		}
	}
	temp_size = temp_size + 10;
	
	memcpy( result_size, &temp_size, 2 );
}

void translate_indirect_to_array ( FILE* disk, short indirect_block_num, short* result_block_num, short array_size ){		

	char* buffer = (char*)calloc(BLOCK_SIZE, 1);
    readBlock(disk, indirect_block_num, buffer, BLOCK_SIZE);
	
	for ( int i = 10 ; i < array_size; i++ ){

		memcpy( &result_block_num[i], buffer + ((i-10)*2), 2 );
	}

}




void create_file_inode ( FILE* disk, int free_block_num, short* file_block_num_array, int size_of_array, int size_of_file ){

	char* inode = calloc(BLOCK_SIZE, 1);
	int flags = 1 ;   							 // 1 = file, 2 = directory
	short dataBlock[12];
	
	for ( int i = 0 ; i < 12 ; i++ ){				// initialize each dataBlock[] = -1
		dataBlock[i] = -1;
		memcpy(inode+8+(i*2), &dataBlock[i], 2);
	}

	//printf( "\n\n testing file_block_num_array : %d \n\n\n" , file_block_num_array[10] );
	//printf( "\n\n testing size_of_array : %d \n\n\n" , size_of_array );
	
	if ( size_of_array < 11 ){
		
		for ( int i = 0 ; i < size_of_array ; i++ ){
			dataBlock[i] = file_block_num_array[i];
			memcpy(inode+8+(i*2), &dataBlock[i], 2);
		}
		
		memcpy(inode, &size_of_file, 4);
		memcpy(inode+4, &flags, 4);
		
		short single_indirect = -1; 
		short double_indirect = -1; 
		
		memcpy(inode+28, &single_indirect, 2);
		memcpy(inode+30, &double_indirect, 2);	
		
		writeBlock(disk, free_block_num, inode, BLOCK_SIZE);
	}
	else{

		for ( int i = 0 ; i < 10 ; i++ ){
			dataBlock[i] = file_block_num_array[i];
			memcpy(inode+8+(i*2), &dataBlock[i], 2);
		}

			short single_indirect_block_num = -1;
			
			create_single_indirect ( disk, file_block_num_array, size_of_array, &single_indirect_block_num );

		memcpy(inode, &size_of_file, 4);
		memcpy(inode+4, &flags, 4);
		
		short single_indirect = single_indirect_block_num; 
		short double_indirect = -1; 
		
		memcpy(inode+28, &single_indirect, 2);
		memcpy(inode+30, &double_indirect, 2);	
		
		writeBlock(disk, free_block_num, inode, BLOCK_SIZE);
		
	}
	free(inode);
}


void create_file_single_inode( FILE* disk, int free_block_num, int dir_block_num  ){

	char* inode = calloc(BLOCK_SIZE, 1);
	int size_of_file = 0 ;
	int flags = 1 ;    							// 1 = file, 2 = directory
	short dataBlock[12];
	
	for ( int i = 0 ; i < 12 ; i++ ){				// initialize each dataBlock[] = -1
		dataBlock[i] = -1;
		memcpy(inode+8+(i*2), &dataBlock[i], 2);
	}

    short dataBlock_0	= (short) dir_block_num;       // the block for this directory
	
	memcpy(inode, &size_of_file, 4);
	memcpy(inode+4, &flags, 4);
	memcpy(inode+8, &dataBlock_0, 2);

	writeBlock(disk, free_block_num, inode, BLOCK_SIZE);
	
	free(inode);
}	

	
void create_directory_inode( FILE* disk, int free_block_num, int dir_block_num  ){

	char* inode = calloc(BLOCK_SIZE, 1);
	int size_of_file = 0 ;
	int flags = 2 ;    							// 1 = file, 2 = directory
	short dataBlock[12];
	
	for ( int i = 0 ; i < 12 ; i++ ){				// initialize each dataBlock[] = -1
		dataBlock[i] = -1;
		memcpy(inode+8+(i*2), &dataBlock[i], 2);
	}

    short dataBlock_0	= (short) dir_block_num;       // the block for this directory
	
	memcpy(inode, &size_of_file, 4);
	memcpy(inode+4, &flags, 4);
	memcpy(inode+8, &dataBlock_0, 2);

	writeBlock(disk, free_block_num, inode, BLOCK_SIZE);
	
	free(inode);
}	



void read_inode ( FILE* disk, int inode_block_num, short* result_block_num ){		// we should get back an array of block num

	char* buffer = (char*)calloc(32, 1);
    readBlock(disk, inode_block_num, buffer, 32);

	for ( int i = 0 ; i < 12 ; i++ ){

		memcpy( &result_block_num[i], buffer + (8 + 2*i), 2 );
	}
}




void create_directory_block ( FILE* disk, int free_block_num2 ){
	
	char* data = (char*)calloc(BLOCK_SIZE, 1);
	//char file_name[] = "file_name_testing";
	//unsigned char inode_number;
	//memcpy( data, &inode_number, 1);
	//strncpy( data+1, name, 31);

	writeBlock(disk, free_block_num2 , data, BLOCK_SIZE);

	free(data);
}



void extend_parent_dir_block ( FILE* disk, int parent_dir_node_block_num, int* new_block_num ){
	
	int free_block_num3;
	
	read_free_block_vector( disk , &free_block_num3 );
	create_directory_block( disk, free_block_num3 );
	fill_free_block_vector( disk , free_block_num3 );
	
	memcpy ( new_block_num, &free_block_num3, 4 );

	int latest_datablock;
	short result_block_num[12];
	
	read_inode( disk, parent_dir_node_block_num, result_block_num );

	for ( int i = 0 ; i < 10 ; i++ ){

		if ( result_block_num[i] != -1 ){
		
			latest_datablock = i;
		}
	}
	
	result_block_num[latest_datablock+1] = (short) free_block_num3;

	char* inode = calloc(32, 1);
    readBlock(disk, parent_dir_node_block_num, inode, 32);				
	memcpy(inode+8+((latest_datablock+1)*2), &result_block_num[latest_datablock+1], 2);
	writeBlock(disk, parent_dir_node_block_num, inode, 32);

	free(inode);
}




void edit_parent_dir_block ( FILE* disk, int parent_dir_node_block_num, int child_dir_inode_num, char* child_dir_name ){
	
	int latest_datablock;
	short result_block_num[12];
	
	read_inode( disk, parent_dir_node_block_num, result_block_num );

	for ( int i = 0 ; i < 10 ; i++ ){
		if ( result_block_num[i] != -1 ){
			latest_datablock = i;
		}
	}
	//printf("\n\n\n\n   *****latest_datablock: %d   \n\n\n\n", latest_datablock );  
	
	char* buffer7 = (char*)calloc(BLOCK_SIZE, 1);	
	readBlock(disk, result_block_num[latest_datablock], buffer7, BLOCK_SIZE);  

	int entry_num = -1; 
	
	for ( int i = 0 ; (i < 16)&&(entry_num == -1) ; i++ ){
		
		unsigned char temp_inode_number; 
		memcpy( &temp_inode_number, buffer7 + (i*32), 1); 
		
		int temp_inode_number2 = (int) temp_inode_number; 

		if ( temp_inode_number2 == 0 ){
			entry_num = i;
		}
	}
	
	if ( entry_num == -1 ){			// full entry
		
		printf("\n\n\n\n   *****extending directory block of parent   \n\n\n\n" );
		
		int new_block_num;
		extend_parent_dir_block ( disk, parent_dir_node_block_num, &new_block_num );	// create new block+link it up to parent inode
		
		unsigned char inode_number = (char) child_dir_inode_num;

		memcpy( buffer7 + (entry_num*32), &inode_number, 1);			//child_dir_inode_num = new build dir inode#
		strncpy( buffer7 + (entry_num*32) +1, child_dir_name, 31);

		writeBlock(disk, new_block_num , buffer7, BLOCK_SIZE);
	}
	
	else{
		unsigned char inode_number = (char) child_dir_inode_num;

		memcpy( buffer7 + (entry_num*32), &inode_number, 1);
		strncpy( buffer7 + (entry_num*32) +1, child_dir_name, 31);

		writeBlock(disk, result_block_num[latest_datablock] , buffer7, BLOCK_SIZE);
	}
	free(buffer7);
}




void del_entry_dir_block ( FILE* disk, int parent_dir_block_num, int del_file_inode_num ){
	
	char* buffer7 = (char*)calloc(BLOCK_SIZE, 1);	
	readBlock(disk, parent_dir_block_num, buffer7, BLOCK_SIZE);

	int entry_num = -1;
	
	for ( int i = 0 ; (i < 16)&&(entry_num == -1) ; i++ ){
		
		unsigned char temp_inode_number;
		memcpy( &temp_inode_number, buffer7 + (i*32), 1);
		
		int temp_inode_number2 = (int) temp_inode_number;
		//printf("*****testing: %d\n", temp_inode_number2 );
		
		if ( temp_inode_number2 == del_file_inode_num ){
			entry_num = i;
		}
	}
	//printf("*****testing deleteing_entry_num: %d\n", entry_num );

	char* temp_entry_buffer = (char*)calloc(32, 1);	
	memcpy( buffer7 + (entry_num*32), temp_entry_buffer, 32 );
	writeBlock(disk, parent_dir_block_num , buffer7, BLOCK_SIZE);
	
	free(buffer7);
}



void search_file_or_dir ( FILE* disk, int dir_block_num, char* file_name, int* finding_inode_num ){

	int entry_num = -1;

	char* buffer3 = (char*)calloc(BLOCK_SIZE, 1);
	readBlock(disk, dir_block_num, buffer3, BLOCK_SIZE);
	
	for ( int i = 0 ; (i < 16)&&(entry_num == -1) ; i++ ){

		char* buffer4 = (char*)calloc(31, 1);	
		strncpy( buffer4, buffer3 + (i*32) + 1 , 30 );

		if ( strncmp( buffer4, file_name , 30 ) == 0 ){
			
			entry_num = i;
			//printf("@@@@@@@@@ matching entry_num: %d\n", entry_num );
		}
		free( buffer4 ); 
	}
	
	if ( entry_num != -1 ){
		
		unsigned char temp_inode_num_hello;
		memcpy ( &temp_inode_num_hello, buffer3 + (entry_num*32), 1 );

		int temp_inode_num_hello2 = (int) temp_inode_num_hello;
		//printf("***********checking for inode#: %d\n", temp_inode_num_hello2 );
		memcpy ( finding_inode_num, &temp_inode_num_hello2, 4 );
	
	} else {
		
		int temp_inode_num_hello2 = -1;
		//printf( "\n\n      no such directory or file in this datablock: '%s'   \n\n", file_name );
		memcpy ( finding_inode_num, &temp_inode_num_hello2, 4 );
	}
	
	free( buffer3 );
	
}


void create_file_block ( FILE* disk, int free_block_num2, char* file_content_buffer ){

	writeBlock(disk, free_block_num2 , file_content_buffer, BLOCK_SIZE);

}


void create_root ( FILE* disk ){
	
	int free_block_num2;
	read_free_block_vector( disk , &free_block_num2 );
	fill_free_block_vector( disk , free_block_num2 );

	int root_dir_block_num = free_block_num2;     
	int free_block_num;
	read_free_block_vector( disk , &free_block_num );
	create_directory_inode( disk, free_block_num, root_dir_block_num );
	fill_free_block_vector( disk , free_block_num );
	
	int next_free_inode_index2;
	find_next_free_inode_num( disk, &next_free_inode_index2 );
	//printf("***********************testing find_next_free_inode #root: %d\n", next_free_inode_index2);
	
	add_mapping( disk, next_free_inode_index2, free_block_num );	// root dir inode = #1,  ( #01, block #25 )
	
	root_inode_index = next_free_inode_index2;
	update_superblock ( disk );
}