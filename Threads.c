/*
 Homework III, Operating Systems
 Fall, 2014
 William Rory Kronmiller
 RIN : 661033063
 */

#include "Dependencies.h"

#if TRACK_THREADS
//Thread tracker struct
struct thread_tracker
{
	int arr_size;
	int index;
	pthread_t ** arr;
};

//Initialize thread tracker
void init_tracker(struct thread_tracker * p_track)
{
	int array_size;
	
	array_size = 2 + EXPECTED_NUM_THREADS;
	
	p_track->index = 0;
	
	p_track->arr_size = array_size;
	
	p_track->arr = malloc(sizeof(pthread_t*)*array_size);
	
	//Error-checking
	if (p_track->arr == NULL)
	{
		perror("Malloc failure in thread tracker operation");
		exit(EXIT_FAILURE);
	}
}

//Append to thread tracker
void add_thread(struct thread_tracker * p_track, pthread_t * new_thread)
{
	//Size-check
	if (p_track->index + 1 >= p_track->arr_size)
	{
		p_track->arr_size += EXPECTED_NUM_THREADS;
		
		if (realloc(p_track->arr, sizeof(pthread_t*)*p_track->arr_size) == NULL)
		{
			perror("Realloc failure in thread tracker operation");
			exit(EXIT_FAILURE);
		}
	}
	
	//Add thread pointer to array
	p_track->arr[p_track->index++] = new_thread;
}
#endif