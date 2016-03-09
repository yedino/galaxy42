/*
This file is educational purposes only! probably has bugs and so on.
*/

/*
A trivial socket client sending data.
*/



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFFER_SIZE 10000

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct hostent *server;

    char buffer[BUFFER_SIZE];

     if (argc < 4) {
         fprintf(stderr,"ERROR wrong params. Usage: program ip portnumber ipfamily\nexample:\nprogram 127.0.0.1 9000 ipv4\nprogram  [ipv6] 9000 ipv6\n");
         exit(1);
     }

	int mode_ipv6 = 0==strcmp("ipv6",argv[3]);  // ***
	printf("IPv6 mode: %s" , (mode_ipv6 ? "YES, IPv6" : "no, ipv4"));

    	if (mode_ipv6) {
    struct sockaddr_in6	serv_addr;
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname2(argv[1], AF_INET6);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin6_family = AF_INET6;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin6_addr.s6_addr,
         server->h_length);
    serv_addr.sin6_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
 

    	}
        else
        {
    struct sockaddr_in serv_addr;
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
        }

    memset(buffer, 'a', 256);
    buffer[BUFFER_SIZE - 1] = '\0';
    while (1) {
    	n = write(sockfd, buffer, strlen(buffer));
	    if (n < 0) 
        	 error("ERROR writing to socket");
    }
    close(sockfd);
    return 0;
}
