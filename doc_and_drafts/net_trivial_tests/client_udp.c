/* UDP client in the internet domain */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 10000

void error(const char *);
int main(int argc, char *argv[])
{
	int sock, n;
	unsigned int length;
	struct sockaddr_in server;
	struct sockaddr_in6 server6;
	struct hostent *hp;
	char buffer[BUFFER_SIZE];

	if (argc != 4) { printf("ERROR wrong params. Usage: program ip portnumber ipfamily\nexample:\nprogram 127.0.0.1 9000 ipv4\nprogram  [ipv6] 9000 ipv6\n");
						  exit(1);
	}

	int mode_ipv6 = 0==strcmp("ipv6",argv[3]);  // ***
	printf("IPv6 mode: %s" , (mode_ipv6 ? "YES, IPv6" : "no, ipv4"));
	if (mode_ipv6) {
		sock= socket(AF_INET6, SOCK_DGRAM, 0);
		if (sock < 0) error("socket");
		server6.sin6_family = AF_INET6;
		hp = gethostbyname2(argv[1], AF_INET6);
		if (hp==0) error("Unknown host");
		bcopy((char *)hp->h_addr,
		  (char *)&server6.sin6_addr,
			hp->h_length);
		server6.sin6_port = htons(atoi(argv[2]));
		length=sizeof(struct sockaddr_in6);
	}

	else {
		sock= socket(AF_INET, SOCK_DGRAM, 0);
		if (sock < 0) error("socket");

		server.sin_family = AF_INET;
		hp = gethostbyname(argv[1]);
		if (hp==0) error("Unknown host");

		bcopy((char *)hp->h_addr,
		  (char *)&server.sin_addr,
			hp->h_length);
		server.sin_port = htons(atoi(argv[2]));
		length=sizeof(struct sockaddr_in);
	}
	memset(buffer, 'a', BUFFER_SIZE);
	while (1) {
		if (mode_ipv6) {
			n=sendto(sock,buffer,
					strlen(buffer),0,(const struct sockaddr *)&server6,length);
		}
		else {
			n=sendto(sock,buffer,
					strlen(buffer),0,(const struct sockaddr *)&server,length);
		}
		if (n < 0) error("Sendto");
	}
	close(sock);
	return 0;
}

void error(const char *msg)
{
	 perror(msg);
	 exit(0);
}
