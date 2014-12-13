/*
 Homework III, Operating Systems
 Fall, 2014
 William Rory Kronmiller
 RIN : 661033063
 */

int init_dir();

void create_file(char*);

void append_file(char*, void*, unsigned int);

int delete_file(char *);

int read_file_formatted_to_fd(char *, int *);