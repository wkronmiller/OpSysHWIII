/*
 Homework III, Operating Systems
 Fall, 2014
 William Rory Kronmiller
 RIN : 661033063
 */

//Command Parsing Functions==========================================

#include "Dependencies.h"

//Tokenizes ADD and APPEND commands
int tokenize_add_append(char ** h_msg, char ** h_file_name, int * p_file_size)
{
	char * scanner, * msg;
	
	//De-reference handle
	msg = *h_msg;
	
	log_debug("Tokenizing Add/Append command");
	
	//Start tokenizing message
	scanner = msg;
	
	//Read past command token
	while(*scanner != ' ')
	{
		//Error-checking
		if(*scanner == '\0')
		{
			//Prematurely reached end of message
			return EXIT_FAILURE;
		}
		
		scanner++;
	}
	
	//Error-checking
	if(*scanner == '\0')
	{
		return EXIT_FAILURE;
	}
	
	//Chop message
	*scanner = '\0';
	
	//Read to beginning of file-name
	scanner++;
	
	//Error-checking
	if(*scanner == '\0')
	{
		return EXIT_FAILURE;
	}
	
	log_debug("Successfully tokenized first part of message");
	log_debug(scanner);
	
	//Point the file-name handle to the scanner
	*h_file_name = scanner;
	
	//Read to end of file-name
	while(*scanner != ' ')
	{
		//Error-checking
		if(*scanner == '\0')
		{
			return EXIT_FAILURE;
		}
		
		scanner++;
	}
	
	//Chop message
	*scanner = '\0';
	
	//Read to beginning of message-length
	scanner++;
	
	//Error-checking
	if(*scanner == '\0')
	{
		return EXIT_FAILURE;
	}
	
	log_debug("Successfully tokenized second part of message");
	log_debug(scanner);
	
	//Set file size
	*p_file_size = atoi(scanner);
	
	log_debug("Successfully converted message length to integer");
	
	return EXIT_SUCCESS;
}

//Tokenizes READ and DELETE commands
int tokenize_read_delete(char ** h_msg, char ** h_file_name)
{
	char * scanner, * msg;
	
	//De-reference handle
	msg = *h_msg;
	
	log_debug("Tokenizing Add/Append command");
	
	//Start tokenizing message
	scanner = msg;
	
	//Read past command token
	while(*scanner != ' ')
	{
		//Error-checking
		if(*scanner == '\0')
		{
			//Prematurely reached end of message
			return EXIT_FAILURE;
		}
		
		scanner++;
	}
	
	//Error-checking
	if(*scanner == '\0')
	{
		return EXIT_FAILURE;
	}
	
	//Chop message
	*scanner = '\0';
	
	//Read to beginning of file-name
	scanner++;
	
	//Error-checking
	if(*scanner == '\0')
	{
		return EXIT_FAILURE;
	}
	
	log_debug("Successfully tokenized first part of message");
	log_debug(scanner);
	
	//Point the file-name handle to the scanner
	*h_file_name = scanner;
	
	return EXIT_SUCCESS;
}