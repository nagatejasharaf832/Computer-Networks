/*
 *  Here is the starting point for your netster part.3 includes. Add the
 *  appropriate comment header as defined in the code formatting guidelines.
 */

#ifndef P3_H
#define P3_H
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

/* add function prototypes */
void stopandwait_server(char*, long, FILE*);
void stopandwait_client(char*, long, FILE*);

#endif
