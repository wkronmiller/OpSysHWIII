/*
 Homework III, Operating Systems
 Fall, 2014
 William Rory Kronmiller
 RIN : 661033063
 */

#include "Dependencies.h"

#define MAX_BUFFERED_CLIENTS 10

#define EXPECTED_NUM_THREADS 100

#define MAX_COMMAND_LEN 24

#define TRACK_THREADS 0

//Print executable help dialog
void show_help(const char * exe_name)
{
	printf("Error: Invalid execution arguments.\n");
	printf("\tUsage %s [port number]\n\tPort Number must be 8000-9000\n", exe_name);
}

//Attempt to parse command-line arguments
int parse_input(int argc, char ** argv)
{
	int portnum; 
	
	//Error-checking
	if (argc != 2)
	{
		if (argc > 0)
		{
			show_help(argv[0]);
		}
				return EXIT_FAILURE;
	}
	
	//Attempt to convert second argument to integer
	portnum = atoi(argv[1]);
	
	//Basic Error-Checking
	if (portnum < 8000 || portnum > 9000)
	{
		fprintf(stderr, "Error: specified port number out of range.\n");
		show_help(argv[0]);
		return EXIT_FAILURE;
	}
	
	return portnum;
}

//Close socket connection and kill thread
void disconnect(int * fd)
{
	log_debug("Disconnect called");
	
	//Kill bad connection
	close(*fd);
	
	log_debug("Client disconnected. Killing thread");
	
	//Kill thread
	pthread_exit(NULL);
	
	//Should never get called
	log_debug("Thread not killed");
}

//Spawn listener socket
int start_listener(unsigned short port_num)
{
	int file_descriptor,
	func_ret;
	struct sockaddr_in sck_set;
	
	//Configure socket settings
	sck_set.sin_family = AF_INET;
	sck_set.sin_addr.s_addr = INADDR_ANY;
	sck_set.sin_port = htons(port_num);
	
	//Create Socket
	file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	
	//Error-checking
	if (file_descriptor < 0)
	{
		fprintf(stderr, "ERROR: Unable to create listener socket\n");
		exit(EXIT_FAILURE);
	}
	
	try_bind:
	//Bind Socket
	func_ret = bind(file_descriptor, (struct sockaddr*) &sck_set, sizeof(sck_set));
	
	//Print output
	printf("Started file-server\nListening on port %u\n", port_num);
	
	//Error-checking
	if (func_ret < 0)
	{
		fprintf(stderr, "ERROR: unable to bind listener socket\n");
		sleep(10);
		goto try_bind;
	}
	
	//Return file descriptor
	return file_descriptor;
}

//Helper function to print commands
void send_commands(int * fd)
{
	int write_size,
	expected_write_size;
	char * output;
	
	//Generate message
	output = "File Storage Server Commands: ADD, APPEND, READ, LIST, DELETE\n";
	
	expected_write_size = strlen(output)*sizeof(char);
	
	//Send message to client
	write_size = write(*fd, output, expected_write_size );
	log_send(output);
	
	//Error-checking
	if (write_size < expected_write_size)
	{
		perror("Socket write failed");
		fflush(stderr);
	}
	
}

void delete_op(char * command, int * fd)
{
	char * ack, * filename;
	int lock_index;
	
	log_debug("Delete operation called");
	
	//Parse message
	if(tokenize_read_delete(&command, &filename) != EXIT_SUCCESS)
	{
		ack = "ERROR: unable to parse command";
		write((*fd), ack, strlen(ack)*sizeof(char));
		log_send(ack);
		
		//Abort
		return;
	}
	
	//Lock array element
	if((lock_index = lock_delete_file_start(filename)) < 0)
	{
		//File not found
		ack = "ERROR: attempting to delete a nonexistent file\n";
		write((*fd), ack, strlen(ack)*sizeof(char));
		log_send(ack);
		
		//Abort
		return;
	}
	
	//Call file-level operation
	if(delete_file(filename) != EXIT_SUCCESS)
	{
		//Filesystem operation failure
		ack = "ERROR: delete operation failed at filesystem level. Check server permissions\n";
		write((*fd), ack, strlen(ack)*sizeof(char));
		log_send(ack);
	}
	
	//Send acknowledgement
	ack = "ACK";
	write((*fd), ack, strlen(ack)*sizeof(char));
	
	//Unlock array
	lock_delete_file_end(lock_index);
}

void list_op(int * fd)
{
	log_debug("List operation called");
	
	lock_list_elems(*fd);
}

void read_op(char * command, int * fd)
{
	char * message, * filename;
	int lock_index;
	
	log_debug("Read operation called");
	
	//Parse message
	if(tokenize_read_delete(&command, &filename) != EXIT_SUCCESS)
	{
		message = "ERROR: unable to parse command\n";
		write((*fd), message, strlen(message)*sizeof(char));
		log_send(message);
		
		//Abort
		return;
	}
	
	//Lock read element
	if((lock_index = lock_file_read_start(filename)) < 0)
	{
		//Element doesn't exist
		message = "ERROR: file not found\n";
		
		write((*fd), message, strlen(message)*sizeof(char));
		log_send(message);
		
		//Abort
		return;
	}
	
	//Attempt to read data
	if(read_file_formatted_to_fd(filename, fd) != EXIT_SUCCESS)
	{
		//Read error has occured
		message = "ERROR: a read error has occured\n";
		
		write((*fd), message, strlen(message)*sizeof(char));
		log_send(message);
		
		//Abort
		return;
	}
	
	//Unlock element
	lock_file_end(lock_index);
}

//Handles add and append operations (opcode = 0 for add, opcode = 1 for append)
void add_append_op(char* info, int * fd, unsigned short int opcode)
{
	void * data;
	char * file_name, * client_message, * ack;
	int file_size, lock_index;
	
	if(opcode == 0)
	{
		log_debug("Add operation called");
	}
	else if(opcode == 1)
	{
		log_debug("Append operation called");
	}
	else
	{
		log_error("Invalid opcode set on add_append_op");
		exit(EXIT_FAILURE);
	}
	
	log_debug(info);
	
	//Call tokenizer
	if(tokenize_add_append(&info, &file_name, &file_size) != EXIT_SUCCESS)
	{
		client_message = "ERROR: command garbled\n";
		
		//Report error to client
		write((*fd), client_message, strlen(client_message)*sizeof(char));
		log_send(client_message);
		
		log_error("Client sent garbled message. Killing connection");
		
		//Kill connection to client
		disconnect(fd);
	}
	
	log_debug("Allocating data memory");
	
	//Allocate memory
	if((data = malloc((size_t)file_size + 3)) == NULL)
	{
		log_error("malloc() failed");
		exit(EXIT_FAILURE);
	}
	
	int read_len = 0;
	
#if 0
	//Read data into buffer
	if((read_len = read((*fd), data, file_size)) != file_size)
	{
		//Read error has occured
		log_error("Received message of invalid length");
		
		disconnect(fd);
	}
#endif
	//Read data into buffer
	do
	{
		read_len += read((*fd), data + read_len, file_size-read_len);
		
	}while(read_len != file_size);
	
	log_debug("Data read from stream. Creating file");
	
	//ADD -specific operations
	if(opcode == 0)
	{
		if((lock_index = lock_add_file_start(file_name)) == -1)
		{
			//Overrride acknowledgement
			ack = "ERROR: file already exists\n";
			write((*fd), ack, strlen(ack)*sizeof(char));
			log_send(ack);
			
			//Cleanup and skip command
			free(data);
			return;
		}
		
		//Create file
		create_file(file_name);
		
		log_debug("File generated. Appending to file");
	}
	else if(opcode == 1) //Append-specific operations
	{
		lock_index = append_lock_elem_start(file_name);
		
		//Error-checking
		if(lock_index < 0)
		{
			//Override ack
			ack = "ERROR: file does not exist\n";
			write((*fd), ack, strlen(ack)*sizeof(char));
			log_send(ack);
			
			//Cleanup and skip command
			free(data);
			return;
		}
		
	}
	else
	{
		//Unrecognized opcode
		log_error("Unrecognized opcode");
		
		exit(EXIT_FAILURE);
	}
	
	//Write data to file
	append_file(file_name, data, file_size);
	
	//Free memory
	free(data);
	
	log_debug("ADD operation complete");
	
	//Set acknowledgement message
	ack = "ACK"; //Deliberately leaving out newline, to conform to directions
	
	//Send acknowledgement
	write((*fd), ack, strlen(ack)*sizeof(char));
	log_send(ack);
	
	//Unlock file
	lock_add_append_file_end(lock_index);
}

//Determine which command to execute
int analyze_command(char * message, int * fd)
{
	int message_len;
	char * msg_not_found = "ERROR: Command Not Found\n",
	* msg_invalid_cmd = "ERROR: Invalid command\n";
	
	log_debug("Analyzing command");
	
	//Get message length
	message_len = strlen(message);
	
	//Basic error-checking
	if (message_len < strlen("LIST"))
	{
		fprintf(stderr, "%s", msg_invalid_cmd);
		write((*fd), msg_invalid_cmd, strlen(msg_invalid_cmd)*sizeof(char));
		log_send(msg_invalid_cmd);
		return EXIT_FAILURE;
	}
	
	//Check if command is add
	if (message_len > strlen("ADD") && strncmp(message, "ADD", 3) == 0)
	{
		//MATCH
		add_append_op(message, fd, 0);
	}
	else if (message_len > strlen("APPEND") && strncmp(message, "APPEND", 6) == 0)
	{
		//MATCH
		add_append_op(message, fd, 1);
	}
	else if (message_len > strlen("READ") && strncmp(message, "READ", 4) == 0)
	{
		//MATCH
		read_op(message, fd);
	}
	else if (message_len >= strlen("LIST") && strncmp(message, "LIST", 4) == 0)
	{
		//MATCH
		list_op(fd);
	}
	else if (message_len > strlen("DELETE") && strncmp(message, "DELETE", 6) == 0)
	{
		//MATCH
		delete_op(message, fd);
	}
	else
	{
		//ERROR: no match
		log_error("Invalid command sent");
		write((*fd), msg_not_found, strlen(msg_not_found)*sizeof(char));
		log_send(msg_not_found);
		return EXIT_FAILURE;
	}
		
	return EXIT_SUCCESS;
}

//Function to parse client commands
int parse_client_command(int * fd)
{
	char s_buffer[2048];
	int read_len,
	s_buf_idx;
	
	s_buf_idx = 0;
	
	//Ensure buffer is clean
	memset(s_buffer, 0, sizeof(s_buffer));
	
	log_debug("Client parse loop called");
		
	do
	{
		//Attempt to read from socket
		read_len = read(*fd, &s_buffer[s_buf_idx], 1);
		
		if (read_len == 0)
		{
			//Client has closed socket
			log_message("Client closed its socket....terminating");
			return 0;
		}
		
		//Check message length
		if (s_buf_idx >= MAX_COMMAND_LEN)
		{
			log_error("ERROR: Invalid command message length");
			close(*fd);
			return 0;
		}
				
	} while (s_buffer[s_buf_idx++] != '\n');
	
	log_debug("Client message newline found, stripping line endings");
	
	//Strip off newline characters
	if (s_buffer[s_buf_idx - 1] == '\n' || s_buffer[s_buf_idx - 1] == '\r')
	{
		s_buffer[s_buf_idx - 1] = '\0';
	}
	if (s_buffer[s_buf_idx - 2] == '\n' || s_buffer[s_buf_idx - 2] == '\r')
	{
		s_buffer[s_buf_idx - 2] = '\0';
	}
	
	log_receipt(s_buffer);
	
	//Call client-command handler
	if(analyze_command(s_buffer, fd) != EXIT_SUCCESS)
	{
		log_debug("parse_client_command has detected that the client command could not be understood");
		
		//Kill bad connection
		disconnect(fd);
	}
	
	//Continue looping
	return 1;
}

//Per-connection thread
void * client_thread(void * in_data)
{
	int file_descriptor;
	
	file_descriptor = *((int*)in_data);
	
	//Send command list to client
	send_commands(&file_descriptor);
	
	//Recieve client command
	while (parse_client_command(&file_descriptor)){}
	
	return (void *)EXIT_SUCCESS;
}

//Start accepting client connections
void accept_clients(int srv_fd)
{
	int fn_ret,
	str_size,
	client_fd;
	struct sockaddr_in client_settings;
	struct hostent *p_host_info;
	
	#if TRACK_THREADS
	
	struct thread_tracker threads;
	
	//Initialize thread tracker
	init_tracker(&threads);
	
	#endif
	
	//Enable connections
	fn_ret = listen(srv_fd, MAX_BUFFERED_CLIENTS);
	
	//Error-checking
	if (fn_ret == -1)
	{
		perror("Unable to enable client connections");
		exit(EXIT_FAILURE);
	}
	
	//Get size of struct
	str_size = sizeof(struct sockaddr_in);
			
	while (1)
	{
		pthread_t new_thread;
		
		//Accept client connection
		client_fd = accept(srv_fd, (struct sockaddr *)&client_settings, (socklen_t*)&(str_size));
		
		//Resolve client hostname
		p_host_info = gethostbyaddr((char*)&client_settings.sin_addr.s_addr, sizeof(struct in_addr), AF_INET);
		
		//Error-checking
		if(p_host_info == NULL)
		{
			printf("Received incoming connection from unknown host\n");
		}
		else
		{
			printf("Received incoming connection from %s\n", p_host_info->h_name);
		}
		
		//Spawn new thread
		if (pthread_create(&new_thread, NULL, (void*)&client_thread, (void*)&client_fd) != 0)
		{
			perror("Thread spawn error");
			exit(EXIT_FAILURE);
		}
		
		#if TRACK_THREADS
		
		//Add thread struct to list
		add_thread(&threads, &new_thread);
						
		#endif
						
		//TODO clean up dead threads
	}
}

//Initialization function
void init()
{
	int exists;

	log_debug("Initializing locks");
	
	//Initialize locks
	init_locks();
	
	log_debug("Initializing storage directory");
	
	//Initialize storage directory
	exists = init_dir(); 
	
	if(exists == 1)
	{
		log_debug("Directory already exists");
	}
}

//Main Function
int main(int argc, char * argv[])
{
	int port_num, //TCP Port Number
	server_fd; //File Descriptor for listener socket
	
	//Parse arguments, with error-checking
	port_num = parse_input(argc, argv);
	
	//State whether debugging is turned on
	log_debug("Debugging is turned on");
			
	if (port_num == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}
	
	//Call initializer helper function
	init();
			
	//Start listener socket
	server_fd = start_listener((unsigned short)port_num);
					
	//Start accepting connections
	accept_clients(server_fd);
					
	return EXIT_SUCCESS;
}