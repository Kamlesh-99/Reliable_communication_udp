/*
Group 48
Sayantan Saha(19CS30041)
Kamalesh Garnayak(19CS10074)
*/

#include "rsocket.h"

#define PORT_M1 50082
#define PORT_M2 50083

int main()
{
	int fd;
	//fd = r_socket(AF_INET,SOCK_MRP,0);
	struct sockaddr_in M1,M2;
	if((fd = r_socket(AF_INET,SOCK_MRP,0))<0){
		perror("Error in socket creation");
		exit(1);
	}	
	//receiver details
	M2.sin_family = AF_INET;
	M2.sin_addr.s_addr = INADDR_ANY;
	M2.sin_port = htons(PORT_M2);
	// Bind the socket
	if(r_bind(fd,(const struct sockaddr *)&M2,sizeof(M2))<0){
		perror("Bind error");
		exit(1);
	}
	printf("Receiver starts running\n");
	char c;
	int length;
	//receive characters in loop
	for(;;){
		length = sizeof(M1);
		if(r_recvfrom(fd,&c,1,0,(struct sockaddr *)&M1,&length)<0){
			printf("Error in receiving.");exit(0);
		}
		printf("%c",c);
		fflush(stdout);
	}
	r_close(fd);
	return 0;
}