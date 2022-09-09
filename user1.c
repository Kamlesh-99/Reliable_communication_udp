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
	char input[100];
	printf("Enter the input string:\n");
	scanf("%s",input);

	int fd = r_socket(AF_INET,SOCK_MRP,0);
	struct sockaddr_in M1,M2;
	if(fd<0){
		perror("Error in socket creation");
		exit(1);
	}
	//ender details
	M1.sin_family = AF_INET;
	M1.sin_addr.s_addr = INADDR_ANY;
	M1.sin_port = htons(PORT_M1);
	//receiver details
	M2.sin_family = AF_INET;
	M2.sin_addr.s_addr = INADDR_ANY;
	M2.sin_port = htons(PORT_M2);
	// Bind the socket
	if(r_bind(fd,(const struct sockaddr *)&M1,sizeof(M1))<0){
		perror("Bind error");
		exit(1);
	}
	printf("Sender starts running\n");
	int i;
	int length = strlen(input);
	// Send each character in loop
	while(1){
		if(i==length)break;
		if(r_sendto(fd,&input[i],1,0,(struct sockaddr *)&M2,sizeof(M2))<0){
			perror("Error in sending");
			exit(1);
		}
		i++;
	}
	r_close(fd);
	return 0;
}