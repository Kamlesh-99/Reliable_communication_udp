/*
Group 48
Sayantan Saha(19CS30041)
Kamalesh Garnayak(19CS10074)
*/

#ifndef RSOCKET_H
#define RSOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

#define T 2
#define PROB 0.15
#define SOCK_MRP 200

typedef struct {
	char msg[210];
	struct sockaddr_in src_addr;
	int msg_len;
}recv_msg;

typedef struct {
	int id;
	char msg[210];
	int msg_len;
	struct sockaddr_in dest_addr;
	time_t time;
}unack_msg;

int dropMessage(float p);
int r_socket(int domain, int type, int protocol);
int r_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t r_sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t r_recvfrom(int sockfd, void *buf, size_t len, int flags,struct sockaddr *src_addr, socklen_t *addrlen);
int r_close(int fd);
void* thread_R();
void* thread_S();


void Receive(int sockfd,char* buffer,const struct sockaddr * src_addr,int msg_len);


#endif