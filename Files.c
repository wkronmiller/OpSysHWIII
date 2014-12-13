/*
 Homework III, Operating Systems
 Fall, 2014
 William Rory Kronmiller
 RIN : 661033063
 */

//File Operations==============================================

#include "Dependencies.h"

#define STORAGE_LOCATION ".storage"

#define STORAGE_PERMISSIONS S_IRUSR | S_IWUSR | S_IXUSR

#define CLOBBER_DIR 0

#define SLOW_WRITE 0
#define SLOW_READ 0

#define DUMP_HX 1


//Generates lock data for existing files in storage directory
void prep_existing()
{
	int lock_index;
	DIR * storagedir;
	struct dirent * dstr;
	
	//Open directory
	if((storagedir = opendir (STORAGE_LOCATION)) == NULL)
	{
		log_error("Unable to open directory. Check permissions");
		perror("Error");
		
		exit(EXIT_FAILURE);
	}
	
	//Get directory data
	while(storagedir && (dstr = readdir(storagedir)))
	{
		//Check if "file" is just . or .."
		if(strlen(dstr->d_name) == 1)
		{
			if(strcmp(dstr->d_name, ".") == 0)
			{
				log_debug("Entry is . skipping");
				continue;
			}
		}
		else if(strlen(dstr->d_name) == 2)
		{
			if(strcmp(dstr->d_name, "..") == 0)
			{
				log_debug("Entry is .. skipping");
				continue;
			}
		}
		//Add file to structure
		log_debug("File found in directory. Adding file to structure");
		
		if((lock_index = lock_add_file_start(dstr->d_name)) < 0)
		{
			log_error("Error adding existing file to lock structure");
			
			exit(EXIT_FAILURE);
		}
		lock_add_append_file_end(lock_index);
	}
	
	//Close directory
	if(closedir(storagedir) != 0)
	{
		log_error("Unable to close directory stream");
		perror("Error");
		
		exit(EXIT_FAILURE);
	}
}

//Initialize storage directory (returns 0 for created, 1 for exists)
int init_dir()
{
	//TODO: parse existing files into lock repository
	
#if CLOBBER_DIR
	log_message("Warning: Clobber_Dir is set to true. Erasing any existing storage directory");
	system("rm -rf .storage");
#endif
	
	//Make sure errno is blanked out
	errno = 0;
	
	//Attempt to make directory
	if(mkdir(STORAGE_LOCATION, STORAGE_PERMISSIONS) == 0)
	{
		//Successfully created new directory
		return 0;
	}
		  
	 //Check if directory already exists
	if(errno == EEXIST)
	{
		log_debug("Directory exists, running prep_existing");
		prep_existing();
		log_debug("Prep_existing complete");
		return 1;
	}
	//Something else went wrong
	perror("Unable to create directory to store files.");
	
	//Server cannot function, exit
	exit(EXIT_FAILURE);
}

//Creates file path from storage directory and file name
char * generate_path(char * file_name)
{
	char * path;
	int path_len;
	
	//Get path length
	path_len = strlen(file_name);
	path_len += strlen(STORAGE_LOCATION);
	path_len++;
	
	//Allocate memory for path
	path = malloc(sizeof(char)*path_len + 10); 
	
	if(sprintf(path, "%s/%s", STORAGE_LOCATION, file_name) != path_len)
	{
		fprintf(stderr, "ERROR: unable to generate file path\n");
		fprintf(stderr, "\tExpected length: %d\n\tActual length: %d\n", path_len, (int)strlen(path));
		fprintf(stderr,"\tMessage: '%s'\n", path);
		exit(EXIT_FAILURE);
	}
	
	//Remember to de-allocate when done
	return path;
}

//Removes file from server directory
int delete_file(char * file_name)
{
	char * file_path;
	
	//Generate file path
	file_path = generate_path(file_name);
	
	if(access(file_path, W_OK) != 0)
	{
		fprintf(stderr, "ERROR: file inaccessible. Check permissions\n");
		free(file_path);
		return EXIT_FAILURE;
	}
	
	if(remove(file_path) != 0)
	{
		perror("ERROR: cannot delete file from system");
		free(file_path);
		return EXIT_FAILURE;
	}
	
	//Log Deletion to console
	log_del(file_name);
	
	free(file_path);
	return EXIT_SUCCESS;
}

//File creation is done in one operation, while writes are done in another
void create_file(char * file_name) //Function either works, or something has gone horribly wrong
{
	char * file_path;
	FILE * file_ptr;
	
	//Generate file path from file name
	file_path = generate_path(file_name);
	
	//Error-checking with access() to ensure file doesn't already exist
	if(access(file_path, F_OK) == 0)
	{
		//File already exists
		fprintf(stderr, "ERROR: attempting to create a file that already exists");
		exit(EXIT_FAILURE);
	}
	
	//File open operation
	if((file_ptr = fopen(file_path, "w")) == NULL)
	{
		perror("ERROR: failed to create file");
		exit(EXIT_FAILURE);
	}
	
	//File close operation
	if(fclose(file_ptr) != 0)
	{
		perror("ERROR: unable to close file.");
		exit(EXIT_FAILURE);
	}
	
	//Cleanup
	free(file_path);
	
	log_debug("Created file");
	log_debug(file_name);
}

//Appends to existing file (called by ADD and APPEND)
void append_file(char* file_name, void * data, unsigned int data_size)
{
	FILE * file_ptr;
	char * file_path;
	
	log_debug("Appending to file");
	log_debug(file_name);
	
	file_path = generate_path(file_name);
	
	//Error-checking: ensure file truly exists
	if(access(file_path, W_OK) != 0)
	{
		fprintf(stderr, "ERROR: unable to append to file %s\n", file_path);
		free(file_path);
		exit(EXIT_FAILURE);
	}
		
	//File open operation
	if((file_ptr = fopen(file_path, "a")) == NULL)
	{
		perror("ERROR: failed to open file");
		free(file_path);
		exit(EXIT_FAILURE);
	}

#if SLOW_WRITE
	//Debugging test to increase potential for race conditions
	log_debug("Doing slow write. Sleeping");
	sleep(10);
#endif
	
	//Write operation
	if(fwrite(data, sizeof(char), (size_t)data_size, file_ptr) != (size_t)data_size)
	{
		perror("ERROR: unable to write to file");
		exit(EXIT_FAILURE);
	}
	
	log_debug("Appending complete");
	
	log_xfer(data_size);
	
	//Cleanup
	free(file_path);
	
	//File close operation
	if(fclose(file_ptr) != 0)
	{
		perror("ERROR: unable to close file.");
		exit(EXIT_FAILURE);
	}
}

//Special file-reading operation
int read_file_formatted_to_fd(char * file_name, int * fil_desc)
{
	int ofd, //Output file descriptor
	read_bytes,
	sent_bytes,
	total_sent,
	total_bytes,
	payload_siz;
	char * file_path, sock_buffer[50];
	FILE * file_ptr;
	char * payload;
	
	//De-reference pointer
	ofd = *fil_desc;
	
	//Generate file path
	file_path = generate_path(file_name);

	//Error-checking: ensure file truly exists
	if(access(file_path, W_OK) != 0)
	{
		fprintf(stderr, "ERROR: unable to read from file %s\n", file_path);
		free(file_path);
		exit(EXIT_FAILURE);
	}
	
	//File open operation
	if((file_ptr = fopen(file_path, "rb")) == NULL)
	{
		perror("ERROR: failed to open file");
		free(file_path);
		exit(EXIT_FAILURE);
	}
	
	sent_bytes = total_bytes = total_sent = read_bytes = 0;
	
	payload_siz = 100000;
	
	if((payload = malloc(payload_siz)) == NULL)
	{
		log_error("Malloc operation failed on file read payload");
		exit(EXIT_FAILURE);
	}
	
	
	while((read_bytes = fread(payload + total_bytes, sizeof(char), 2048, file_ptr)) != 0)
	{
		
#if SLOW_READ
		log_debug("SLOW_READ activated. Sleeping");
		
		//Debugging feature to increase potential for race conditions
		sleep(20);
#endif
		
		//Count bytes
		total_bytes += read_bytes;
		
		//Dynamic expansion of payload
		if(total_bytes+3000 >= payload_siz)
		{
			payload_siz += 10000;
			
			char * payload_tmp;
			
			if((payload_tmp = malloc(payload_siz)) == NULL)
			{
				log_error("Unable to expand payload to fit incoming data stream");
				exit(EXIT_FAILURE);
			}
			
			memcpy(payload_tmp, payload, payload_siz - 10000);
			
			free(payload);
			
			payload = payload_tmp;
		}
	}
	
	//Generate ack header
	sprintf(sock_buffer, "ACK %u\n", total_bytes);
	
	//Write ack to header
	if(write((ofd), sock_buffer, sizeof(char)*strlen(sock_buffer)) != (sizeof(char)*strlen(sock_buffer)))
	{
		log_error("ACK send error");
		exit(EXIT_FAILURE);
	}
	
	log_send(sock_buffer);
	
	//Write data to socket
	while((sent_bytes = write((ofd), payload + total_sent, total_bytes - total_sent)) != 0)
	{
		total_sent += sent_bytes;
	}
	
	//Error-checking
	if(total_bytes != total_sent)
	{
		log_error("Entire file not sent");
	}
	
	log_xfer(total_bytes);
	
	//File close operation
	if(fclose(file_ptr) != 0)
	{
		perror("ERROR: unable to close file.");
		exit(EXIT_FAILURE);
	}
	
	return EXIT_SUCCESS;
}