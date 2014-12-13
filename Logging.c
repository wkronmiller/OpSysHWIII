/*
 Homework III, Operating Systems
 Fall, 2014
 William Rory Kronmiller
 RIN : 661033063
 */

#include "Dependencies.h"

#define LOG_ERROR 1
#define LOG_DEBUG 0
#define ERROR_FILEDES stderr

//Log file deletion
void log_del(char * filename)
{
	char buffer[2048];
	
	//Delete "abc.txt" file
	sprintf(buffer, "Delete \"%s\" file", filename);
	
	//Send to printer
	log_message(buffer);
}

//Print File Transfer
void log_xfer(int size)
{
	char buffer[100];
	
	sprintf(buffer, "Transferred file (%u bytes)", size);
	
	//Send to printer
	log_message(buffer);
}

//Print rcvd message
void log_receipt(char * message)
{
	int len_message;
	char * output_buffer, * header;
	
	header = "Rcvd: "; //Do we need to null-terminate?
	
	len_message = strlen(message);
	
	len_message += strlen(header);
	
	//Allocate with error-checking
	if((output_buffer = malloc(sizeof(char)*(len_message+10))) == NULL)
	{
		log_error("Unable to allocate mmeory for rcvd message");
		exit(EXIT_FAILURE);
	}
	
	//Construct message
	sprintf(output_buffer, "%s%s", header, message);
	
	//Send to printer
	log_message(output_buffer);
	
	//Memory cleanup
	free(output_buffer);
}

void log_send(char * message)
{
	int len_message;
	char * output_buffer, * header;
	
	header = "Sent: ";
	
	len_message = strlen(message);
	
	len_message += strlen(header);
	
	//Allocate with error-checking
	if((output_buffer = malloc(sizeof(char)*(len_message+10))) == NULL)
	{
		log_error("Unable to allocate mmeory for rcvd message");
		exit(EXIT_FAILURE);
	}
	
	//Construct message
	sprintf(output_buffer, "%s%s", header, message);
	
	//Send to printer
	log_message(output_buffer);
	
	//Memory cleanup
	free(output_buffer);
	//TODO
}

//Print log message
void print_formatted(char * message)
{
	printf("[thread %u] %s\n", (unsigned int)pthread_self(), message);
}

//Log to Console
void log_message(char * message)
{
	unsigned int tid;
	tid = (unsigned int) pthread_self();
	fprintf(stdout, "[thread %u] %s\n", tid, message);
	fflush(stdout);
}

//Log Error Messages
void log_error(char * message)
{
#if LOG_ERROR
	unsigned int tid;
	tid = (unsigned int) pthread_self();
	fprintf(ERROR_FILEDES, "[thread %u] ERROR: %s\n", tid, message);
	fflush(ERROR_FILEDES);
#endif
}

//Log Debug Messages
void log_debug(char * message)
{
#if LOG_DEBUG
	unsigned int tid;
	tid = (unsigned int) pthread_self();
	fprintf(DEBUG_FILDES, "[thread %u] DEBUG: %s\n", tid, message);
	fflush(DEBUG_FILDES);
#endif
}