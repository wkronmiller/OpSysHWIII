/*
 Homework III, Operating Systems
 Fall, 2014
 William Rory Kronmiller
 RIN : 661033063
 */

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>
#include <dirent.h>

#define DEBUG_FILDES stdout
#define LOG_DEBUG 0

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0


#include "Logging.h"
#include "Files.h"
#include "Parsers.h"
#include "Locks.h"