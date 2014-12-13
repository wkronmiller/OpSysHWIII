/*
 Homework III, Operating Systems
 Fall, 2014
 William Rory Kronmiller
 RIN : 661033063
 */

//Mutex and Semaphore Lock Functions==============================================

#include "Dependencies.h"
#include "Locks.h"
#include <unistd.h>

static struct Lock_Arr_Struct arr_str;

//Simulate a sem wait on element
void elem_wait(struct Lock_Struct * p_elem)
{
	log_debug("Waiting on element lock");
	
	//Locks element
	pthread_mutex_lock(&(p_elem->elem_change_lock));
	
	log_debug("Element locked");
	
	//Error-checking
	if(p_elem->num_readers < 0)
	{
		log_error("Element reader count negative");
	}
	
	//Update reader count
	p_elem->num_readers++;
	
	log_debug("elem num_readers incremented");
	
	//Unlocks element
	pthread_mutex_unlock(&(p_elem->elem_change_lock));
	
	log_debug("Element unlocked");
}

//Simulate a sem post on element
void elem_post(struct Lock_Struct * p_elem)
{
	log_debug("Waiting on element lock");
	
	//Locks element
	pthread_mutex_lock(&(p_elem->elem_change_lock));
	
	log_debug("Element locked");
	
	//Error-checking
	if(p_elem->num_readers <= 0)
	{
		log_error("Element reader count negative");
	}
	
	//Update reader count
	p_elem->num_readers--;
	
	log_debug("elem num_readers decremented");
	
	//Unlocks element
	pthread_mutex_unlock(&(p_elem->elem_change_lock));
	
	log_debug("Element unlocked");
}

//Function to wait for an element to free up, then lock it
int elem_lockfree(struct Lock_Struct * p_elem)
{
	int sleep_count;
	
	log_debug("Element Lockfree called");
	
	while(1)
	{
		//Lock element
		pthread_mutex_lock(&(p_elem->elem_change_lock));
		
		if(p_elem->modifying == 0)
		{
			log_debug("Modifying bit is zero");
			//Set modifying bit
			p_elem->modifying = 1;
			log_debug("Modifying bit set to 1");
			
			pthread_mutex_unlock(&(p_elem->elem_change_lock));

			break;
		}
		
		pthread_mutex_unlock(&(p_elem->elem_change_lock));
		
		usleep(10);
	}
	
	log_debug("Waiting for readers to leave");
	
	sleep_count = 0;
	
//Jump location
elem_waitfree:
	
	//We will have already locked the array, by now, so new readers aren't a problem
	while(p_elem->num_readers > 1)
	{
		//Spin
		sleep(1);
		sleep_count++;
		
		if(sleep_count > 15)
		{
			log_error("Waitfree has been waiting for a long time.");
		}
		else if(sleep_count > 50)
		{
			//Lock element
			pthread_mutex_lock(&(p_elem->elem_change_lock));
			
			log_debug("Element lock established");
			
			//Set modifying bit
			p_elem->modifying = 0;
			
			log_debug("Modifying bit set to zero");
			
			//Unlock element
			pthread_mutex_unlock(&(p_elem->elem_change_lock));
			
			log_debug("Possible deadlock condition detected. Backing out");
			elem_post(p_elem);
			return EXIT_FAILURE;
		}
	}
	
	//Error-checking
	if(p_elem->num_readers == 0)
	{
		log_error("Number of readers should be 1");
	}
	
	//Lock element
	pthread_mutex_lock(&(p_elem->elem_change_lock));
	
	log_debug("Element lock established");
	
	//Ensure a reader didn't slip in
	if(p_elem->num_readers > 1)
	{
		//A reader started on this element before we could lock it
		//This should not happen
		log_debug("A reader got in. Trying again");
		
		//Unlock mutex
		pthread_mutex_unlock(&(p_elem->elem_change_lock));
		
		//Try again
		goto elem_waitfree;
	}
	
	//Set num_readers
	p_elem->num_readers = 1;
	
	log_debug("Number of readers set");
	
	return EXIT_SUCCESS;
}

//Function to unlock an element
void elem_unlock(struct Lock_Struct * p_elem)
{
	log_debug("Unlocking element");
	
	//Decrement num_readers
	p_elem->num_readers = 0;
	
	//Set modifying bit
	p_elem->modifying = 0;
	
	log_debug("Modifying bit set to zero");
	
	//Unlock mutex
	pthread_mutex_unlock(&(p_elem->elem_change_lock));
}


//Function to simulate a semaphore wait (sort of)
void arr_wait()
{
	//Locks entire array
	pthread_mutex_lock(&(arr_str.array_lock));
	
	log_debug("Entire array locked");
	
	//Locks the array struct
	pthread_mutex_lock(&(arr_str.struct_lock));
	
	log_debug("Array struct locked");
	
	//Unlocks entire array
	pthread_mutex_unlock(&(arr_str.array_lock));
	
	log_debug("Array unlocked");	
	
	//Error-checking
	if(arr_str.num_readers < 0)
	{
		log_error("Array reader count is negative");
	}
	
	//Updates reader count
	arr_str.num_readers++;
	
	log_debug("Reader count updated");
	
	//Unlocks the array struct
	pthread_mutex_unlock(&(arr_str.struct_lock));
	
	log_debug("Array struct unlocked");
}

//Function to simulate a semaphore post (sort of)
void arr_post()
{
	//Locks array struct
	pthread_mutex_lock(&(arr_str.struct_lock));
	
	log_debug("Array struct locked");
	
	//Error-checking
	if(arr_str.num_readers <= 0)
	{
		log_error("Array reader count is negative");
	}
	
	//Updates reader count
	arr_str.num_readers--;
	
	log_debug("Array num_readers decremented");
	
	//Unlocks array struct
	pthread_mutex_unlock(&(arr_str.struct_lock));
	
	log_debug("Array struct unlocked");
}

//Initialize Lock_Struct element
void init_lock_elem(struct Lock_Struct * p_elem, char * fil_name)
{
	//Initialize mutexes
	pthread_mutex_init(&(p_elem->elem_change_lock), NULL);
	log_debug("elem_change_lock initialized");
	
	//Lock the mutexes
	pthread_mutex_lock(&(p_elem->elem_change_lock));
	log_debug("elem_change_lock locked");
	
	//Update the reader count
	p_elem->num_readers = 1;
	
	//Initialize modification int
	p_elem->modifying = 0;
	
	log_debug("num_readers initialized to 1");
	
	if(fil_name != NULL)
	{
		//Initialize file name
		p_elem->file_name = malloc(sizeof(char)*strlen(fil_name) + 10);
		
		//Set file name
		strcpy(p_elem->file_name, fil_name);
	}
	else
	{
		p_elem->file_name = NULL;
	}
	
	//Set current operation
	//p_elem->current_op = 3;
}

//Function to increase array size by ten elements
int arr_incr_siz(char * fil_name)
{
	int index, ret_idx;
	
	log_debug("Waiting for array lock");
	
	//Locks entire array (preventing new readers)
	pthread_mutex_lock(&(arr_str.array_lock));
	
	log_debug("Waiting for array to empty out");
	
	while(arr_str.num_readers > 1) //Calling function should have waited
	{
		//SPIN
	}
	
	//Locks array structure
	pthread_mutex_lock(&(arr_str.struct_lock));
	
	log_debug("Array struct locked. Increasing array size");
	
	//Resize array
	arr_str.alloc_size += 10;
	arr_str.array_size ++;
	
	if((arr_str.p_lock_array = realloc(arr_str.p_lock_array, sizeof(struct Lock_Struct)*(arr_str.alloc_size))) == NULL)
	{
		perror("Array resize failed");
		exit(EXIT_FAILURE);
	}
	
	//Initialize new elements
	ret_idx = index = arr_str.alloc_size - 10;
	
	init_lock_elem(&(arr_str.p_lock_array[index++]), fil_name);
	
	while(index < arr_str.alloc_size)
	{
		init_lock_elem(&(arr_str.p_lock_array[index]), NULL);
		
		(arr_str.p_lock_array[index]).num_readers = 0;
		pthread_mutex_unlock(&((arr_str.p_lock_array[index]).elem_change_lock));
		
		index++;
	}
	
	//Set file name
	arr_str.p_lock_array[ret_idx].file_name = malloc(sizeof(char)*strlen(fil_name)+10);
	strcpy(arr_str.p_lock_array[ret_idx].file_name, fil_name);
	
	//Unlock struct
	pthread_mutex_unlock(&(arr_str.struct_lock));
	
	//Unlock entire array
	pthread_mutex_unlock(&(arr_str.array_lock));
	
	log_debug("Entire array unlocked");
	
	//Return index of "new" element
	return ret_idx;
}

//Function to decrement array size safely
void arr_decr_siz()
{	
	int sleep_count;
	
	log_debug("Waiting for array to empty out");
	
	sleep_count = 0;
	
	while(arr_str.num_readers > 1) //Calling function should have waited
	{
		usleep(100);
		
		if(sleep_count > 1 && sleep_count % 100)
		{
			log_debug("Still waiting for readers to leave");
		}
		//SPIN
	}
	
	//Locks array structure
	pthread_mutex_lock(&(arr_str.struct_lock));
	
	log_debug("Array struct locked. Decrementing array size value");
	
	//Resize array
	arr_str.array_size --;
	
	//Unlock struct
	pthread_mutex_unlock(&(arr_str.struct_lock));
	
	//Unlock entire array
	pthread_mutex_unlock(&(arr_str.array_lock));
	
	log_debug("Entire array unlocked");
}

//Search array for specified element (returns -1 if not found)
int find_elem(char * file_name, int release_lock)
{
	int num_elems, index;
	
	//NOTE: Ensure array is waited/posted by calling function
	
	//De-reference pointers to needed data
	num_elems = arr_str.alloc_size;
	
	//Initialize index counter
	index = 0;
	
	//Search through array
	while(index < num_elems)
	{
check_match:
		//Wait on element
		elem_wait(&(arr_str.p_lock_array[index]));
		if((file_name != NULL) && (arr_str.p_lock_array[index].file_name != NULL))
		{
			if(strlen(file_name) == strlen(arr_str.p_lock_array[index].file_name))
			{
				if(strcmp(file_name, arr_str.p_lock_array[index].file_name) == 0)
				{
					log_debug("File lock element found");
					
					//Check if file is being modified
					if(arr_str.p_lock_array[index].modifying != 0)
					{
						log_debug("find_elem has found element, but element is being modified. Sleeping and re-checking");
						
						elem_post(&(arr_str.p_lock_array[index]));
						
						log_debug("elem_post called");
						
						usleep(100);
						
						goto check_match;
					}
					
					if(release_lock)
					{
						elem_post(&(arr_str.p_lock_array[index]));
					
						log_debug("elem_post called");
					}
					
					//Return index of found element
					return index;
				}
			}
			
#if PRINT_ARRAY_ELEMENTS
			log_debug(arr_str.p_lock_array[index].file_name);
#endif
		}
		else if((file_name == NULL) && (arr_str.p_lock_array[index].file_name == NULL))
		{
			//Check if file is being modified
			if(arr_str.p_lock_array[index].modifying != 0)
			{
				log_debug("find_elem has found element, but element is being modified. Sleeping and re-checking");
				
				elem_post(&(arr_str.p_lock_array[index]));
				
				log_debug("elem_post called");
				
				usleep(100);
				
				goto check_match;
			}
			
			if(release_lock)
			{
				elem_post(&(arr_str.p_lock_array[index]));
			
				log_debug("elem_post called");
			}
			
			//Return index of free element
			return index;
		}
		
		//Post on element
		elem_post(&(arr_str.p_lock_array[index]));
		
		log_debug("elem_post called");
		
		index++;
	}
	
	log_debug("File lock element NOT found");
	
	//Element not found
	return -1;
}

//Add new file to lock array (returns nonzero if element already exists)
int add_elem(char * file_name)
{
	int new_idx;
	
	//NOTE: ensure array is wait/post'ed by calling function
	
//Jump point
try_add:
	//Try to find a free element in the array
	if((new_idx = find_elem(NULL, 0)) == -1)
	{
		log_debug("Unable to find a free element in array. Expanding array");
		
		//Expand array
		new_idx = arr_incr_siz(file_name);
	}
	else
	{
		//Lock element
		while(elem_lockfree(&(arr_str.p_lock_array[new_idx])) != EXIT_SUCCESS){}
		log_debug("elem_lockfree called");
		
		//Ensure element is usable
		if(arr_str.p_lock_array[new_idx].file_name != NULL)
		{
			log_debug("Element no longer null");
			
			//Unlock element
			elem_unlock(&(arr_str.p_lock_array[new_idx]));
			log_debug("elem_unlock called");
			
			//Try again
			goto try_add;
		}
		
		//Allocate space for name
		if((arr_str.p_lock_array[new_idx].file_name = malloc(sizeof(char)*(strlen(file_name) + 10))) == NULL)
		{
			log_error("Memory allocation failed");
			
			exit(EXIT_FAILURE);
		}
		
		//Set name
		strcpy(arr_str.p_lock_array[new_idx].file_name, file_name);
		
		//Increment array size count
		pthread_mutex_lock(&(arr_str.struct_lock));
		
		arr_str.array_size++;
		
		pthread_mutex_unlock(&(arr_str.struct_lock));
		
	}
	
	//NOTE: ensure calling function unlocks element
	
	//Return index of new element
	return new_idx;
}

//Delete Lock Struct Element
void delete_lock_elem(int index)
{
	char * p_name;
	
	//Call array wait
	arr_wait(arr_str);
	
	//Lock array, preventing new accessors
	pthread_mutex_lock(&(arr_str.array_lock));
	
	//Get pointer to element
	struct Lock_Struct * elem;
	
	elem = &(arr_str.p_lock_array[index]);
	
	//Lock element when free
	while(elem_lockfree(elem) != EXIT_SUCCESS){}
	log_debug("elem_lockfree called");
	
	//Get pointer to element's name
	p_name = arr_str.p_lock_array[index].file_name;
	
	//Error-checking
	if(p_name == NULL)
	{
		log_error("Attempting to delete blank element");
		return;
	}
	
	//Free memory for name
	free(p_name);
	
	//Set name pointer to NULL (marking this element as re-usable)
	arr_str.p_lock_array[index].file_name = NULL;
	
	//Decrement array size
	arr_decr_siz();
	
	//Unlock array
	pthread_mutex_unlock(&(arr_str.array_lock));

}

//Exported functions=========================

//Initializes locking data structure
void init_locks()
{
	arr_str.alloc_size = 0;
	arr_str.array_size = 0;
	arr_str.num_readers = 0;
	pthread_mutex_init(&(arr_str.struct_lock), NULL);
	log_debug("struct_lock initialized");
}

//Called by ADD to lock file
int lock_add_file_start(char * file_name)
{
	int index;
	
	//Ensure file doesn't already exist
	if(find_elem(file_name, 1) != -1)
	{
		//File already exists
		log_error("Attempting to add a file that already exists");
		
		return -1;
	}
	
	//Wait on array
	arr_wait();
	
	//Call helper function
	index = add_elem(file_name);
	
	log_debug("New element added for file");
	log_debug(file_name);
	
	return index;
}

//Append an element
int append_lock_elem_start(char * file_name)
{
	int index;
	
append_start:
	//Try to find a free element in the array
	if((index = find_elem(file_name, 0)) == -1)
	{
		log_debug("Unable to find element in lock array");
		
		//Return fail
		return -1;
	}
	
	//Wait on array
	arr_wait();
	
	//Lock element
	while(elem_lockfree(&(arr_str.p_lock_array[index])) != EXIT_SUCCESS){}
	log_debug("elem_lockfree called");
	
	//Ensure we didn't experience a race condition
	if((strlen(file_name) != strlen(arr_str.p_lock_array[index].file_name)) || strcmp(file_name, arr_str.p_lock_array[index].file_name) != 0)
	{
		log_debug("Race condition detected");
		
		elem_unlock(&(arr_str.p_lock_array[index]));
		goto append_start;
	}
	
	return index;
}

//Called by APPEND and ADD functions to free file for other operations
void lock_add_append_file_end(int lock_index)
{
	//Unlock element
	elem_unlock(&(arr_str.p_lock_array[lock_index]));
	log_debug("elem_unlock called");
	
	//Post array
	arr_post();
	
	log_debug("Lock_add_file_end completed");
}

//Called by DELETE at start of operation
int lock_delete_file_start(char * file_name)
{
	int index; 
	
	//Ensure file exists
	if((index = find_elem(file_name, 0)) == -1)
	{
		log_debug("Delete: Unable to find element in lock array");
		
		//Return fail
		return -1;
	}
	
	log_debug("Delete: calling helper function");
	
	//Call Helper function
	delete_lock_elem(index);
	
	log_debug("Delete helper function completed. Element erased");
	
	return index;
}

void lock_delete_file_end(int index)
{	
	//Unlock element
	elem_unlock(&(arr_str.p_lock_array[index]));
	log_debug("elem_post called");
	
	//Post aray
	arr_post();
	log_debug("array_post called");
}

int lock_file_read_start(char * file_name)
{
	int index;
	
	//Ensure file exists
	if((index = find_elem(file_name, 1)) == -1)
	{
		log_debug("Unable to find requested file");
		
		//Return fail
		return -1;
	}
	
	//Wait on array
	arr_wait();
	log_debug("Array wait called");
	
	//Wait on element
	elem_wait(&(arr_str.p_lock_array[index]));
	log_debug("Element wait called");
	
	return index;
}

void lock_file_end(int index)
{
	//Post on element
	elem_post(&(arr_str.p_lock_array[index]));
	log_debug("Element posted");
	
	//Post on array
	arr_post();
	log_debug("Array posted");
}

void lock_list_elems(int fd)
{
	char out_buf[4096];
	int elem_count, index;
	
	elem_count = arr_str.array_size;
	
	sprintf(out_buf, "%u\n", elem_count);
	write(fd, out_buf, sizeof(char)*strlen(out_buf));
	
	//Search array
	index = 0;
	
	while(index < arr_str.alloc_size)
	{
		if(arr_str.p_lock_array[index].file_name != NULL)
		{
			sprintf(out_buf, "%s\n", arr_str.p_lock_array[index].file_name);
			write(fd, out_buf, sizeof(char)*strlen(out_buf));
			elem_count--;
		}
		index++;
	}
	
	while(elem_count-- > 0)
	{
		log_error("Elem-count not accurate");
		
		sprintf(out_buf, "ERROR\n");
		write(fd, out_buf, sizeof(char)*strlen(out_buf));
	}
}