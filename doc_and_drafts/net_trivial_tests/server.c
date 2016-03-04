/*
This file is educational purposes only! probably has bugs and so on.
*/


/*
A trivial socket server receiving data.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#define BUFFER_SIZE 10000

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
		printf("Starting \n");

     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[BUFFER_SIZE];


     if (argc < 3) {
         fprintf(stderr,"ERRPR wrong params. Usage: program portnumber ipfamily\nexample:\nprogram 9000 ipv4\nprogram 9000 ipv6\n");
         exit(1);
     }

		int mode_ipv6 = 0==strcmp("ipv6",argv[2]);  // ***
		fprintf("IPv6 mode: %s" , (mode_ipv6 ? "YES, IPv6" : "no, ipv4"));

		if (mode_ipv6) {
     struct sockaddr_in6 serv_addr, cli_addr;
     sockfd = socket(AF_INET6, SOCK_STREAM, 0);
     if (sockfd < 0)
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin6_family = AF_INET6;
     const struct in6_addr in6addr_any = IN6ADDR_ANY_INIT; //  ...
     serv_addr.sin6_addr = in6addr_any; // ...
     serv_addr.sin6_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd,
                 (struct sockaddr *) &cli_addr,
                 &clilen);
     if (newsockfd < 0)
          error("ERROR on accept");

		}
		else
		{
     struct sockaddr_in serv_addr, cli_addr;
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd,
                 (struct sockaddr *) &cli_addr,
                 &clilen);
     if (newsockfd < 0)
          error("ERROR on accept");
		}

		// count speed etc:
		long long int count_all=0;  // all since start
		long long int w_count_b=0;  // in this window: count of bytes
		long long int w_count_pkt=0;  // in this window: count of packets
		long long int w_time1=0; // start when this window started
		const long long int w_len = 2; // window length in seconds

		w_time1=time(NULL);
		while (w_time1 == time(NULL)) { } ; // a silly way to wait for a new second to start - so that counter starts at exact second
		w_time1=time(NULL);


     while (1) {

     int n;

	bzero(buffer, BUFFER_SIZE);
	n = read(newsockfd, buffer, BUFFER_SIZE -1);

	w_count_b += n; // number of bytes (received)

	//printf("read %d bytes\n", n);
     	if (n < 0) error("ERROR reading from socket");
	//printf("Here is the message: %s\n", buffer);

	// n = write(newsockfd, buffer, n);
	// if (n < 0) error("ERROR writing to socket");
	//sleep(1);

	// if (n != BUFFER_SIZE) printf("Warning, received smaller packet n=%d buf=%d \n", n, BUFFER_SIZE);
	++w_count_pkt;
	++count_all;

	if (1==(count_all % 1000))	{
		// printf("Count: %lld \n", count_all);
	}

	if (1==(count_all % 100))	{
		long long int w_time2 = time(NULL); // now

		if (w_time2 >= w_time1 + w_len) {
			printf("\nStats at count: %lld \n", count_all);

			double speed = w_count_b / ( ((double)w_time2) - w_time1);
			double speed_pkt = w_count_pkt / ( ((double)w_time2) - w_time1);
			double w_avg_pkt_size = w_count_b / ((double)w_count_pkt);

			printf("speed: %f    avg pkt size %f bits (%f bytes)\n", w_avg_pkt_size*8, w_avg_pkt_size);
			printf("speed: %f    pkt  /sec\n", speed_pkt);
			printf("speed: %f    byte /sec\n", speed);
			printf("speed: %f Ki byte /sec\n", speed/1024);
			printf("speed: %f Mi byte /sec\n", speed/(1024*1024));
			printf("speed: %f    bit  /sec\n", 8*speed);
			printf("speed: %f Ki bit  /sec\n", 8*speed/1024);
			printf("speed: %f Mi bit  /sec\n", 8*speed/(1024*1024));

			// restart window counter:
			w_time1 = w_time2;
			w_count_b = 0;
		}
	}

     }

     close(newsockfd);
     close(sockfd);
     return 0;
}
