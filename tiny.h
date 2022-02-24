#ifndef __TINY_H_
#define __TINY_H_

#include <arpa/inet.h> /* inet_ntoa */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifndef LISTENQ
#define LISTENQ 1024 /* second argument to listen() */
#endif

#ifndef MAXLINE
#define MAXLINE 1024 /* max length of a line */
#define NMAXLINE 1023
#endif

#ifndef DEFAULT_PORT
#define DEFAULT_PORT 9999
#endif

#ifndef RIO_BUFSIZE
#define RIO_BUFSIZE 8192
#endif

/* Simplifies calls to bind(), connect(), and accept() */
typedef struct sockaddr SA;

void handle_request(int out_fd, char *file_name);
int open_listenfd(int port);
char *process(int fd, struct sockaddr_in *clientaddr);

#endif
