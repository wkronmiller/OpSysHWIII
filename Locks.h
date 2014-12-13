/*
 Homework III, Operating Systems
 Fall, 2014
 William Rory Kronmiller
 RIN : 661033063
 */

#pragma once

#include "Dependencies.h"

//Lock Struct Array Element (1:1 correspondence to files in storage)
struct Lock_Struct
{
	//Locks array element struct
	pthread_mutex_t elem_change_lock; //Prevents a change to array element
	
	//Tracks readers
	int num_readers;
	
	//Tracks modifications for lockfree
	int modifying;
	
	//Name of the file being pointed to
	char * file_name;
	
	//Current file operation taking place
	//unsigned int current_op;
};

//Structure for storing an array of file mutexes
struct Lock_Arr_Struct
{
	//Locks entire array (for parts of create and delete operations)
	pthread_mutex_t array_lock; //Prevents access to entire array
	pthread_mutex_t struct_lock; //Locks only the structure, not the array
	
	//Tracks readers
	int num_readers;
	
	//Number of valid elements in array
	unsigned int array_size; 
	
	//Number of elements allocated (used+available)
	unsigned int alloc_size;
	
	//Pointer to first element of array
	struct Lock_Struct * p_lock_array;
};

//Exported Functions=========================================

void init_locks();

//Returns -1 if file exists
int lock_add_file_start(char *);

void lock_add_append_file_end(int);

//Returns -1 if file does not exist
int append_lock_elem_start(char *);

//Returns -1 if file does not exist
int lock_delete_file_start(char *);

void lock_delete_file_end(int);

int lock_file_read_start(char *);

void lock_file_end(int);

void lock_list_elems(int);