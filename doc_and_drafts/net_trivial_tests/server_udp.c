/* Creates a datagram server.  The port 
	number is passed as an argument.  This
	server runs forever */

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>

#define BUFFER_SIZE 10000

void error(const char *msg)
{
	 perror(msg);
	 exit(0);
}

int main(int argc, char *argv[])
{
	int sock, length, n;
	socklen_t fromlen;
	struct sockaddr_in server;
	struct sockaddr_in from;
	char buf[BUFFER_SIZE];

	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(0);
	}

	sock=socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) error("Opening socket");
	length = sizeof(server);
	bzero(&server,length);
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=INADDR_ANY;
	server.sin_port=htons(atoi(argv[1]));
	if (bind(sock,(struct sockaddr *)&server,length)<0)
		 error("binding");
	fromlen = sizeof(struct sockaddr_in);
	while (1) {
		 n = recvfrom(sock,buf,BUFFER_SIZE,0,(struct sockaddr *)&from,&fromlen);
		 if (n < 0) error("recvfrom");
		 //write(1,"Received a datagram: ",21);
		 //write(1,buf,n);
		 //n = sendto(sock,"Got your message\n",17,
		//				0,(struct sockaddr *)&from,fromlen);
		 //if (n  < 0) error("sendto");
	}
	return 0;
 }

