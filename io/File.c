
const int BLOCK_SIZE = 512;
const int NUM_BLOCKS = 4096;
const int INODE_SIZE = 32;

int NUM_INODE = 0;
int root_inode_index = -1;
int magic_number = 1;
int mapping_block_start = 9; 	// ( block 9-24 for inode #1 - #2048 )
int x=1;

void writeBlock(FILE* disk, int blockNum, char* data, int size){
    fseek(disk, blockNum * BLOCK_SIZE, SEEK_SET);
    fwrite(data, size, 1, disk); 
}

void readBlock(FILE* disk, int blockNum, char* buffer, int size){
    fseek(disk, blockNum * BLOCK_SIZE, SEEK_SET);
    fread(buffer, size, 1, disk);
}

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


void initLLFS ( FILE* disk ){
	
	//char* init = calloc(BLOCK_SIZE*NUM_BLOCKS, 1);
	//fwrite(init, BLOCK_SIZE*NUM_BLOCKS, 1, disk);
	//free(init);
	
	create_superblock( disk );
	create_free_block_vector( disk );
	create_root ( disk );

	NUM_INODE++;
	update_superblock( disk );
	//read_superblock( disk );
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
	//printf("testing find_next_free_inode #sub: %d\n", next_free_inode_index3);

	add_mapping( disk, next_free_inode_index3, free_block_num4 );		// sub dir inode = #2,  ( #02, block #28 )
	
	NUM_INODE++;
	update_superblock( disk );
	//read_superblock( disk );

	
	//modify parent (root)
	int child_dir_inode_num = next_free_inode_index3;   
	edit_parent_dir_block ( disk, parent_dir_node_block_num, child_dir_inode_num, child_dir_name );
}



void create_file ( FILE* disk, char* file_content, int parent_dir_inode_num, char* file_name  ){
	
	//printf("\n\n &&&&&&&&&&&&&&&&&&&&&& testing length of file_content: %d\n", strlen(file_content) );

	if ( strlen( file_content ) < 512 ){

		int free_block_num4;											// block for file content
		char* file_content_buffer = (char*)calloc(BLOCK_SIZE, 1);
		
		strncpy ( file_content_buffer, file_content, strlen(file_content) );
		//printf("testing file_content: %s\n", file_content_buffer);
		
		read_free_block_vector( disk , &free_block_num4 );
		create_file_block( disk, free_block_num4, file_content_buffer );
		fill_free_block_vector( disk , free_block_num4 );


		int file_block_num = free_block_num4;   
		int free_block_num5;											// block for file inode

		read_free_block_vector( disk , &free_block_num5 );
		create_file_single_inode( disk, free_block_num5, file_block_num );
		fill_free_block_vector( disk , free_block_num5 );

		
		int next_free_inode_index4;
		
		find_next_free_inode_num( disk, &next_free_inode_index4 );
		add_mapping( disk, next_free_inode_index4, free_block_num5 );	// file inode = #3
		
		
		NUM_INODE++;
		update_superblock( disk );										//update superblock info
		//read_superblock( disk );
		
		
		//add to parent inode ( into sub directory )

		int file_inode_num = next_free_inode_index4;   
		edit_parent_dir_block ( disk, parent_dir_inode_num, file_inode_num, file_name );
	}
	else {
		
		int block_needed;
		
		if ( (strlen(file_content) % 512) != 0 ){						// case: if doesnt fill whole block
			block_needed = (strlen(file_content))/512+1 ;
		}else{
			block_needed = (strlen(file_content))/512 ;
		}
			
		short store_block_array[ block_needed ];
		//printf( "\n\n testing block_needed : %d \n\n\n" , block_needed );
			
			for (  int i = 0 ; i < block_needed ; i++ ){

				int free_block_num4;									// block for file content
				
				char* file_content_buffer = (char*)calloc(BLOCK_SIZE, 1);
				strncpy ( file_content_buffer, file_content + (i*512) , 512 );

				read_free_block_vector( disk , &free_block_num4 );
				create_file_block( disk, free_block_num4, file_content_buffer );
				fill_free_block_vector( disk , free_block_num4 );

				store_block_array[i] = (short) free_block_num4;

				//printf("\ntesting store_block_array %d: %d\n", i,  store_block_array[i] );
			}

			int size_of_array = block_needed;
			
			int free_block_num5;			// block for file inode
			int size_of_file = strlen(file_content);

			read_free_block_vector( disk , &free_block_num5 );
			fill_free_block_vector( disk , free_block_num5 );
			create_file_inode( disk, free_block_num5, store_block_array, size_of_array, size_of_file );


			int next_free_inode_index4;
			
			find_next_free_inode_num( disk, &next_free_inode_index4 );
			add_mapping( disk, next_free_inode_index4, free_block_num5 );	// file inode = #3

			
			NUM_INODE++;
			update_superblock( disk );		//update superblock info
			//read_superblock( disk );
			
			
			//add to parent inode ( into sub directory )

			int file_inode_num = next_free_inode_index4;   
			edit_parent_dir_block ( disk, parent_dir_inode_num, file_inode_num, file_name );
	}
}



void create_empty_file ( FILE* disk, int parent_dir_inode_num, char* file_name  ){

		int free_block_num4;											// block for file content
		char* file_content_buffer = (char*)calloc(BLOCK_SIZE, 1);
		
		read_free_block_vector( disk , &free_block_num4 );
		create_file_block( disk, free_block_num4, file_content_buffer );
		fill_free_block_vector( disk , free_block_num4 );


		int file_block_num = free_block_num4;   
		int free_block_num5;											// block for file inode

		read_free_block_vector( disk , &free_block_num5 );
		create_directory_inode( disk, free_block_num5, file_block_num );
		fill_free_block_vector( disk , free_block_num5 );

		
		int next_free_inode_index4;
		
		find_next_free_inode_num( disk, &next_free_inode_index4 );
		add_mapping( disk, next_free_inode_index4, free_block_num5 );	// file inode = #3
		
		
		NUM_INODE++;
		update_superblock( disk );										//update superblock info
		//read_superblock( disk );
		
		
		//add to parent inode ( into sub directory )

		int file_inode_num = next_free_inode_index4;   
		edit_parent_dir_block ( disk, parent_dir_inode_num, file_inode_num, file_name );
}



void open_file ( FILE* disk, char* input ){

	char* parent_directory_name = (char*)calloc(31,1);	
	char* curr_file_name = (char*)calloc(31,1);	
	char* fake_curr_file_name = (char*)calloc(31,1);	
	
	const char s[2] = "/";

	fake_curr_file_name = strtok( input, s );		// skip the first command "open"
		
	strncpy( parent_directory_name, curr_file_name, 31);
	strncpy( curr_file_name, fake_curr_file_name, 31);
	
	fake_curr_file_name = strtok(NULL, s);
	
	int saved_parent_inode_block_num = -1;
	
		int newest_root_inode_index = get_root_inode_index ( disk );
			
		int root_inode_block_num = find_mapping( disk, newest_root_inode_index );

		short result_block_num4[12];
		read_inode( disk, root_inode_block_num, result_block_num4 );
			
		saved_parent_inode_block_num = root_inode_block_num;	

	
	while( fake_curr_file_name != NULL ) {

		strncpy( parent_directory_name, curr_file_name, 31);
		strncpy( curr_file_name, fake_curr_file_name, 31);
		fake_curr_file_name = strtok(NULL, s);
		//printf( "curr_file_name: %s ", curr_file_name );
		//printf( "parent_directory_name: %s\n", parent_directory_name );

		if ( fake_curr_file_name == NULL ){
			
			int checking_inode_num;		
			short result_block_num3[12];
	
			read_inode( disk, saved_parent_inode_block_num, result_block_num3 );
			
			for ( int i = 0 ; result_block_num3[i] != -1 ; i++ ){
			
				search_file_or_dir( disk, result_block_num3[i], curr_file_name, &checking_inode_num );
				
				if ( checking_inode_num > 0 ){
					
					saved_parent_inode_block_num = find_mapping( disk, checking_inode_num );
				}
			}

			if ( checking_inode_num < 1 ){
				printf( "\n      no such directory or file in this datablock: '%s'   \n", curr_file_name );
				break;
			}
			
			short result_block_num[12];
			read_inode( disk, saved_parent_inode_block_num, result_block_num );		// saved_parent=file_inode
			
			if ( result_block_num[10] > 0 ){
				
				//printf( "\n\n testing result_block_num[10] : %d \n\n\n" , result_block_num[10] );
				
				short result_size;
				translate_indirect_to_size( disk, result_block_num[10], &result_size );
				
				//printf( "\n\n testing result_size : %d \n\n\n" , result_size );
				
				short result_block_num5[result_size];
				read_inode( disk, saved_parent_inode_block_num, result_block_num5 );	
				
				//printf( "\n\n testing result_block_num5[10] : %d \n\n\n" , result_block_num5[10] );
				
				translate_indirect_to_array(disk, result_block_num[10], result_block_num5, result_size);
				
				//printf( "\n\n testing result_block_num5[10] : %d \n\n\n" , result_block_num5[10] );
				
				printf("\n\n       content of the file: \n" );
				
				for ( int i = 0 ; (result_block_num5[i] != -1) && (i<result_size) ; i++ ){
					
					char* buffer = (char*)calloc(BLOCK_SIZE,1);	
					readBlock(disk, (int)result_block_num5[i], buffer, BLOCK_SIZE);
					printf("%s", buffer);
						
					free(buffer);
				}
				printf("\n\n\n");
			}
			else{
				printf("\n\n       content of the file: \n" );
				
				for ( int i = 0 ; (result_block_num[i] != -1) && (i<10) ; i++ ){
					
					char* buffer = (char*)calloc(BLOCK_SIZE,1);	
					readBlock(disk, (int)result_block_num[i], buffer, BLOCK_SIZE);
					printf("%s", buffer);
						
					free(buffer);
				}
				printf("\n\n\n");
			}
		}

		else {
		
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
				printf( "\n      no such directory in this datablock: '%s'   \n", curr_file_name );
				break;
			}
		}
	}
	
	free( parent_directory_name );
	free( curr_file_name );
	free( fake_curr_file_name );
	
	
}




void make_directory ( FILE* disk, char* input ){
	
	char* parent_directory_name = (char*)calloc(31,1);	
	char* curr_dir_name = (char*)calloc(31,1);	
	char* fake_curr_dir_name = (char*)calloc(31,1);	
	
	const char s[2] = "/";

	fake_curr_dir_name = strtok( input, s );		// skip the first command "Mkdir"

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
		//printf( "curr_dir_name: %s ", curr_dir_name );
		//printf( "parent_directory_name: %s\n", parent_directory_name );

		if ( fake_curr_dir_name == NULL ){

			create_sub_directory ( disk, saved_parent_inode_block_num, curr_dir_name );

			printf("\n       --> we added a sub directory: %s\n\n", curr_dir_name );
			
		}
		else{

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




void write_file ( FILE* disk, char* input, char* file_content_larger ){
	
	char* parent_directory_name = (char*)calloc(31,1);	
	char* curr_file_name = (char*)calloc(31,1);	
	char* fake_curr_dir_name = (char*)calloc(31,1);	
	
	const char s[2] = "/";

	fake_curr_dir_name = strtok( input, s );		// skip the first command "Mkfile"

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
		//printf( "curr_file_name: %s ", curr_file_name );
		//printf( "parent_directory_name: %s\n", parent_directory_name );

		if ( fake_curr_dir_name == NULL ){
			
			create_file ( disk, file_content_larger, saved_parent_inode_block_num, curr_file_name );

			printf("\n       --> we added a file, file name: %s\n\n", curr_file_name );
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
				printf( "\n      no such directory in this datablock: '%s'   \n", curr_file_name );
				break;
			}
		}
	}
	free( parent_directory_name );
	free( curr_file_name );
	free( fake_curr_dir_name );	
}



void write_empty_file ( FILE* disk, char* input ){
	
	char* parent_directory_name = (char*)calloc(31,1);	
	char* curr_file_name = (char*)calloc(31,1);	
	char* fake_curr_dir_name = (char*)calloc(31,1);	
	
	const char s[2] = "/";

	fake_curr_dir_name = strtok( input, s );		// skip the first command "Mkfile"

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
		//printf( "curr_file_name: %s ", curr_file_name );
		//printf( "parent_directory_name: %s\n", parent_directory_name );

		if ( fake_curr_dir_name == NULL ){
			
			create_empty_file ( disk, saved_parent_inode_block_num, curr_file_name );

			printf("\n       --> we added an empty file, file name: %s\n\n", curr_file_name );
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
				printf( "\n      no such directory in this datablock: '%s'   \n", curr_file_name );
				break;
			}
		}
	}
	free( parent_directory_name );
	free( curr_file_name );
	free( fake_curr_dir_name );	
}


void delete_file ( FILE* disk, int parent_dir_block_num, char* curr_file_name, int del_file_inode_num ){

	// free file block + inode block
	// parent dir data block inside remove file
	// delete inode mapping	
	
	short del_block_num[12];
	int del_inode_block_num = find_mapping( disk, del_file_inode_num );

	read_inode ( disk, del_inode_block_num, del_block_num );

	for ( int i = 0 ; del_block_num[i] != -1 ; i++ ){
		delete_free_block_vector( disk, del_block_num[i] );
	}
	
	delete_free_block_vector( disk, del_inode_block_num );
	NUM_INODE--;
	update_superblock( disk );
	
	delete_mapping ( disk, del_file_inode_num );
	
	del_entry_dir_block ( disk, parent_dir_block_num, del_file_inode_num );
}


void delete_directory ( FILE* disk, int parent_dir_block_num, char* curr_file_name, int del_dir_inode_num ){
	

	// free file block + inode block
	// parent dir data block inside remove file
	// delete inode mapping	
	
	short del_block_num[12];
	int del_inode_block_num = find_mapping( disk, del_dir_inode_num );
	
	read_inode ( disk, del_inode_block_num, del_block_num );
	
	for ( int i = 0 ; del_block_num[i] != -1 ; i++ ){
		delete_free_block_vector( disk, del_block_num[i] );
	}
	
	delete_free_block_vector( disk, del_inode_block_num );
	NUM_INODE--;
	update_superblock( disk );
	
	delete_mapping ( disk, del_dir_inode_num );
	
	del_entry_dir_block ( disk, parent_dir_block_num, del_dir_inode_num );
}




void Rm_file ( FILE* disk, char* input ){
	
	char* parent_directory_name = (char*)calloc(31,1);	
	char* curr_file_name = (char*)calloc(31,1);	
	char* fake_curr_dir_name = (char*)calloc(31,1);	
	
	const char s[2] = "/";

	fake_curr_dir_name = strtok( input, s );		// skip the first command "Mkfile"

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
		//printf( "curr_file_name: %s ", curr_file_name );
		//printf( "parent_directory_name: %s\n", parent_directory_name );

		if ( fake_curr_dir_name == NULL ){
			
			short result_block_num[12];
			read_inode( disk, saved_parent_inode_block_num, result_block_num );

			int checking_inode_num = -1;		
			int file_appear_in_which_block_of_directory;
			
			for ( int i = 0 ; (result_block_num[i] != -1) && ( checking_inode_num == -1 ) ; i++ ){
			
				search_file_or_dir( disk, result_block_num[i], curr_file_name, &checking_inode_num );
				
				if ( checking_inode_num != -1 ){
				
					file_appear_in_which_block_of_directory = i;
				}
			}
			
			if ( checking_inode_num == -1 ){
				printf("file not exist\n");
				break;
			}
			
			delete_file ( disk, result_block_num[file_appear_in_which_block_of_directory], curr_file_name, checking_inode_num );

			printf("\n       --> we deleted a file, file name: %s\n\n", curr_file_name );
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
				printf( "\n      no such directory in this datablock: '%s'   \n", curr_file_name );
				break;
			}
		}
	}
	free( parent_directory_name );
	free( curr_file_name );
	free( fake_curr_dir_name );	
}



void list_file ( FILE* disk, char* input ){
	
	//printf( "****************************%s \n", input );
	
	char* parent_directory_name = (char*)calloc(31,1);	
	char* curr_dir_name = (char*)calloc(31,1);	
	char* fake_curr_dir_name = (char*)calloc(31,1);	
	
	const char s[2] = "/";

	fake_curr_dir_name = strtok( input, s );		// skip the first command "Mkfile"

	strncpy( parent_directory_name, curr_dir_name, 31);
	strncpy( curr_dir_name, fake_curr_dir_name, 31);
	
	fake_curr_dir_name = strtok(NULL, s);

	int saved_parent_inode_block_num = -1;
	
		int newest_root_inode_index = get_root_inode_index ( disk );
	
		int root_inode_block_num = find_mapping( disk, newest_root_inode_index );

		short result_block_num4[12];
		read_inode( disk, root_inode_block_num, result_block_num4 );
	
		saved_parent_inode_block_num = root_inode_block_num;
	
		
		if ( fake_curr_dir_name == NULL ){

				short result_block_num[12];
				read_inode( disk, saved_parent_inode_block_num, result_block_num );	

				printf("\n\n         here is all the file in %s directory: \n\n", curr_dir_name );

				for ( int i = 0 ; (result_block_num[i] != -1) && (i<10) ; i++ ){

					short inode_num_empty = 0;
					char* buffer3 = (char*)calloc(BLOCK_SIZE, 1);
					readBlock(disk, result_block_num[i], buffer3, BLOCK_SIZE);

					for ( int i = 0 ; i < 16 ; i++ ){

						char* buffer4 = (char*)calloc(31, 1);
						
						strncpy( buffer4, buffer3 + (i*32) + 1 , 31 );
						memcpy ( &inode_num_empty, buffer3 + (i*32), 1 );
							
						if ( inode_num_empty != 0 ) {
							
							printf("         #### file name %d : %s\n", i , buffer4 );
						}
						
						free( buffer4 ); 
					}
					free( buffer3 );
				}
				printf("\n       --> we listed all the file in %s directory \n\n", curr_dir_name );
		}


	while( fake_curr_dir_name != NULL ) {

		strncpy( parent_directory_name, curr_dir_name, 31);
		strncpy( curr_dir_name, fake_curr_dir_name, 31);
		fake_curr_dir_name = strtok(NULL, s);
		//printf( "curr_dir_name: %s ", curr_dir_name );
		//printf( "parent_directory_name: %s\n", parent_directory_name );

		if ( fake_curr_dir_name == NULL ){
			
			int checking_inode_num;		
			short result_block_num3[12];
	
			read_inode( disk, saved_parent_inode_block_num, result_block_num3 );
			
			for ( int i = 0 ; result_block_num3[i] != -1 ; i++ ){
			
				search_file_or_dir( disk, result_block_num3[i], curr_dir_name, &checking_inode_num );
				
				if ( checking_inode_num > 0 ){
					
					saved_parent_inode_block_num = find_mapping( disk, checking_inode_num );
				}
			}
			if ( checking_inode_num < 1 ){
				printf( "\n      no such directory in this datablock: '%s'   \n", curr_dir_name );
				break;
			}

			//printf("*********************************************************************\n");

			short result_block_num[12];
			read_inode( disk, saved_parent_inode_block_num, result_block_num );	

			printf("\n\n         here is all the file in %s directory: \n\n", curr_dir_name );

			for ( int i = 0 ; (result_block_num[i] != -1) && (i<10) ; i++ ){

				short inode_num_empty = 0;
				char* buffer3 = (char*)calloc(BLOCK_SIZE, 1);
				readBlock(disk, result_block_num[i], buffer3, BLOCK_SIZE);

				for ( int i = 0 ; i < 16 ; i++ ){

					char* buffer4 = (char*)calloc(31, 1);
					
					strncpy( buffer4, buffer3 + (i*32) + 1 , 31 );
					memcpy ( &inode_num_empty, buffer3 + (i*32), 1 );
						
					if ( inode_num_empty != 0 ) {
						
						printf("         #### file name %d : %s\n", i , buffer4 );
					}
					
					free( buffer4 ); 
				}
				free( buffer3 );
			}
			printf("\n       --> we listed all the file in %s directory \n\n", curr_dir_name );
		}
		else{

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



void Rm_dir ( FILE* disk, char* input ){
	
	char* parent_directory_name = (char*)calloc(31,1);	
	char* curr_dir_name = (char*)calloc(31,1);	
	char* fake_curr_dir_name = (char*)calloc(31,1);	
	
	const char s[2] = "/";
	
	fake_curr_dir_name = strtok( input, s );				// skip the first command "Mkfile"
	
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
		//printf( "curr_dir_name: %s ", curr_dir_name );
		//printf( "parent_directory_name: %s\n", parent_directory_name );

		if ( fake_curr_dir_name == NULL ){
			
			short result_block_num[12];
	
			read_inode( disk, saved_parent_inode_block_num, result_block_num );

			int checking_inode_num = -1;		
			int file_appear_in_which_block_of_directory;
			
			for ( int i = 0 ; (result_block_num[i] != -1) && ( checking_inode_num == -1 ) ; i++ ){
			
				search_file_or_dir( disk, result_block_num[i], curr_dir_name, &checking_inode_num );
				
				if ( checking_inode_num != -1 ){
				
					file_appear_in_which_block_of_directory = i;
				}
			}
			if ( checking_inode_num == -1 ){
				printf( "\n      no such directory: '%s'   \n", curr_dir_name );
				break;
			}
			
			delete_directory ( disk, result_block_num[file_appear_in_which_block_of_directory], curr_dir_name, checking_inode_num );

			printf("\n       --> we deleted a directory, directory name: %s\n\n", curr_dir_name );
		}
		else{

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



void command_input ( FILE* disk, char* input, char* file_content_larger ){

	char tok_string7[10];
	strncpy( tok_string7, input, 6 );
	char* command7 = (char*)calloc(60,1);	
	char s[2] = "/";
	command7 = strtok( tok_string7, s );		// skip the first command "open"
	
	char* input2 = (char*)calloc(60,1);
	strncpy( input2, input, 60 );
	char* path = (char*)calloc(60,1);
	path = strtok( input2, "/" );
	path = strtok( NULL, "" );
	
	if ( file_content_larger != NULL ){
		robust_superblock ( disk, input, strlen(file_content_larger) );
	}else{
		robust_superblock ( disk, input, 0 );
	}
	
	if ( path == NULL ){
		path = "root";
	}
	
	printf("\n\ntest case %d: ", x++ );
	


	if ( strncmp( command7, "list", 4 ) == 0 ){		
	
		printf( "we are listing the file in a directory at '%s'", path );
		list_file( disk, input );
	}
	else if ( strncmp( command7, "Open", 4 ) == 0 ){			// check "Open" command
		
		printf( "we are opening a file at '%s'", path );
		open_file( disk, input );
	}
	else if ( strncmp( command7, "Rmdir", 5 ) == 0 ){		
		
		printf( "we are deleting a directory at '%s'", path );
		Rm_dir( disk, input );
	}
	else if ( strncmp( command7, "Rmfile", 6 ) == 0 ){
		
		printf( "we are deleting a file at '%s'", path );
		Rm_file( disk, input );
	}
	else if ( (strncmp( command7, "Writefile", 6 ) == 0 ) && file_content_larger == NULL ){

		printf( "we are making a new empty file at '%s'", path );
		write_empty_file( disk, input );
		//write_file( disk, input, file_content_larger );
	}
	else if ( strncmp( command7, "Writefile", 6 ) == 0 ){			
		
		printf( "we are making a new file with content at '%s'", path );
		write_file( disk, input, file_content_larger );
	}
	else if ( strncmp( command7, "Mkdir", 5 ) == 0 ){		
		
		printf( "we are making a directory at '%s'", path );
		make_directory( disk, input );
	}

}


/*
int robust_file_exist ( FILE* disk, int saved_parent_inode_block_num, char* curr_file_name ){
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
}
*/

int robust_size( FILE* disk, int last_context_size, int saved_parent_inode_block_num ){

	char* buffer = (char*)calloc(32, 1);
    readBlock(disk, saved_parent_inode_block_num, buffer, 32);

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
    readBlock(disk, 0, buffer, BLOCK_SIZE);

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
			//exit(0);
		} else {
			printf("\n   ****vdisk is up to date, previous compile successfully****\n");
			printf( " -------------------------------------------------------------------- \n\n" );
		}
	}
	free(buffer);
}