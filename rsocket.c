

#include "rsocket.h"


int udp;

recv_msg *received_messgae_table;
int start=0;	
int end=0;

unack_msg* unacknowledged_message_table;
int last = -1;

pthread_t R;
pthread_t S;
pthread_mutex_t mutex;

int id_count = 0;
int tranmission = 0;

int r_socket(int domain, int type, int protocol)
{
	
	if((udp = socket(domain,SOCK_DGRAM,protocol))<0){
		return -1;
	}
	else{
		srand(time(NULL));
		pthread_mutex_init(&mutex,NULL);
		pthread_create(&R,NULL,thread_R,NULL);
		pthread_create(&S,NULL,thread_S,NULL);
		received_messgae_table = (recv_msg*) calloc(120,sizeof(recv_msg));
		unacknowledged_message_table = (unack_msg*) calloc(120,sizeof(unack_msg));
		return udp;
	}
}
// Function to bind the socket to the port - similar to bind call
int r_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	return bind(sockfd,addr,addrlen);
}

// Function to send messages similar to sendto call.
ssize_t r_sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{	
	char *buffer = (char*)malloc(sizeof(char)*210);
	memset(buffer,'\0',sizeof(buffer));
	buffer[0] = 'U';	
	int id_ht = htonl(id_count);
	memcpy(&buffer[1],&id_ht,sizeof(id_ht));
	memcpy(&buffer[sizeof(id_count)+1],buf,len);
	pthread_mutex_lock(&mutex);
	id_count++;			
	last++;	
	unacknowledged_message_table[last].id = id_count-1;
	memcpy(unacknowledged_message_table[last].msg,buffer,len+sizeof(id_count)+1);
	unacknowledged_message_table[last].msg_len = len+sizeof(id_count)+1;
	unacknowledged_message_table[last].dest_addr = *(struct sockaddr_in *)dest_addr;
	unacknowledged_message_table[last].time = time(NULL);
	printf("Transmitted successfully the character at index %d : %s\n",id_count-1,&(buffer[5]));
	int bytes;
	if ((bytes = sendto(sockfd,buffer,len+sizeof(id_count)+1,flags,dest_addr,addrlen))<0){
		perror("Error in sending");
		exit(0);
	}
	tranmission++;		
	pthread_mutex_unlock(&mutex);
	/*if(bytes < 0){
		perror("Error in sending");
	}*/
	return bytes;
}

// Function to read data from the receive buffer similar to recvfrom call
ssize_t r_recvfrom(int sockfd, void *buf, size_t len, int flags,struct sockaddr *src_addr, socklen_t *addrlen)
{
	pthread_mutex_lock(&mutex);
	while(1){
		if(start!=end)break;
		pthread_mutex_unlock(&mutex);
		sleep(1);	
		pthread_mutex_lock(&mutex);
	}
	
	int size;
	if(len<=received_messgae_table[start].msg_len){
		size = len;
	}
	else{
		size = received_messgae_table[start].msg_len;
	}
	memcpy(buf,received_messgae_table[start].msg,size);
	*src_addr = *(struct sockaddr *)&received_messgae_table[start].src_addr;
	*addrlen = sizeof(received_messgae_table[start].src_addr);
	start = (start+1)%120;		
	pthread_mutex_unlock(&mutex);
	return size;
}

//thread R to receive message
void* thread_R()
{
	int sockfd = udp;
	fd_set readfd;
	int nfds = sockfd+1;
	struct timeval tv;
	tv.tv_sec = T;
	tv.tv_usec = 0;
	int val;
	for(;;){
		FD_ZERO(&readfd);
		FD_SET(sockfd,&readfd);
		val = select(nfds,&readfd,NULL,NULL,&tv);
		
		if(val>0){
			
			if(FD_ISSET(sockfd,&readfd)){
				char buffer[210];
				memset(buffer,'\0',sizeof(buffer));
				struct sockaddr_in src_addr;
				int len;
				len = sizeof(src_addr);
				int msg_len;
				if((msg_len = recvfrom(sockfd,buffer,210,0,(struct sockaddr *)&src_addr,&len))<0){
					perror("Error in receiving");
					exit(0);
				}
				if(dropMessage(PROB)==0){
					Receive(sockfd,buffer,(const struct sockaddr *)&src_addr,msg_len);
				}
				else{
					//printf("Dropping the received message\n");
				}
			}
		}
	}
}	

//thread S to retransmit messages
void* thread_S()
{
	int sockfd = udp;
	fd_set readfd;
	int nfds = sockfd+1;
	struct timeval tv;
	tv.tv_sec = T;
	tv.tv_usec = 0;
	int val;
	for(;;){
		FD_ZERO(&readfd);
		FD_SET(sockfd,&readfd);
		val = select(nfds,&readfd,NULL,NULL,&tv);
		
		if(val==0){
			
			int i=0;
			time_t time_now = time(NULL);
			pthread_mutex_lock(&mutex);
			i=0;
			while(i<=last){
				if(time_now - unacknowledged_message_table[i].time >= 2*T){
					if(sendto(sockfd,unacknowledged_message_table[i].msg,unacknowledged_message_table[i].msg_len,0,(const struct sockaddr *)&(unacknowledged_message_table[i].dest_addr),sizeof(unacknowledged_message_table[i].dest_addr))<0){
						perror("Error in sending");
						exit(0);
					}
					printf("Retransmitted successfully the character at index %d : %s\n",unacknowledged_message_table[i].id,&(unacknowledged_message_table[i].msg[5]));
					unacknowledged_message_table[i].time = time_now;
					tranmission++;
				}
				i++;
			}
			pthread_mutex_unlock(&mutex);
			tv.tv_sec = T;
			tv.tv_usec = 0;
		}
		
	}
}

// Function to handle the message received by thread R
void Receive(int sockfd,char* buffer,const struct sockaddr * src_addr,int msg_len)
{
	int i;
	int id;
	memcpy(&id,&buffer[1],sizeof(id));
	id = ntohl(id);
	if(buffer[0] == 'U'){
		pthread_mutex_lock(&mutex);
		
		memcpy(received_messgae_table[end].msg,&buffer[sizeof(id)+1],msg_len);
		received_messgae_table[end].src_addr = *(struct sockaddr_in *)src_addr;
		received_messgae_table[end].msg_len = msg_len;
		end = (end+1)%120;
		pthread_mutex_unlock(&mutex);
		//Sending the acknowledgement
		char buffer1[sizeof(int)+1];
		buffer1[0] = 'A';		
		int id_htonl = htonl(id);
		memcpy(&buffer1[1],&id_htonl,sizeof(id_htonl));
		if(sendto(sockfd,buffer1,sizeof(id_htonl)+1,0,(const struct sockaddr *)src_addr,sizeof(*src_addr))<0){
			perror("Error in sending");
			exit(1);
		}
	}
	else{
		pthread_mutex_lock(&mutex);
		i=0;
		while(i<=last){
			if(unacknowledged_message_table[i].id==id){
				printf("Received ack for character at index %d : %s\n",id,&(unacknowledged_message_table[i].msg[sizeof(id)+1]));
				unacknowledged_message_table[i] = unacknowledged_message_table[last];
				last--;
				break;
			}
			i++;
		}
		pthread_mutex_unlock(&mutex);
	}
}




// Function to determine when to drop a message based on a probability measure
int dropMessage(float p)
{
	float rand_num = (float)rand()/((float)RAND_MAX);
	return (rand_num<p);
}

// Function to close the socket
int r_close(int fd)
{
	pthread_mutex_lock(&mutex);
	while(1){
		if(last<0)break;
		pthread_mutex_unlock(&mutex);
		sleep(1);
		pthread_mutex_lock(&mutex);
	}
	printf("Total transmissions happened: %d\n",tranmission);
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
	free(received_messgae_table);
	free(unacknowledged_message_table);
	pthread_kill(R, SIGKILL);
	pthread_kill(S, SIGKILL);
	return close(fd);
}
