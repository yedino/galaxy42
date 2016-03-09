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
#include <time.h>
#include <strings.h>

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
	//struct sockaddr_in server;
	struct sockaddr_in from;
	struct sockaddr_in6 from6;
	char buf[BUFFER_SIZE];
	int mode_ipv6 = 0; // should we use IPv6 (or else IPv4 is used) - configured from command line

	if (argc != 3) {
		fprintf(stderr, "ERROR wrong params.\nUsage: program portnumber ipfamily\nexample:\nprogram 9000 ipv4\nprogram 9000 ipv6\n");
		exit(0);
	}

	mode_ipv6 = 0==strcmp("ipv6", argv[2]);  // ***
	printf("IPv6 mode: UDP %s\n" , (mode_ipv6 ? "YES" : "no (using IPv4. Try option ipv6 to use IPv6 instead)"));

	if (mode_ipv6) {
		struct sockaddr_in6 server;
		sock=socket(AF_INET6, SOCK_DGRAM, 0);
		if (sock < 0) error("Opening socket");
		server.sin6_family=AF_INET6;
		const struct in6_addr in6addr_any = IN6ADDR_ANY_INIT; //  ...
		server.sin6_addr=in6addr_any;
		server.sin6_port=htons(atoi(argv[1]));
		length = sizeof(server);
		if (bind(sock,(struct sockaddr *)&server,length)<0)
			error("binding");
		fromlen = sizeof(struct sockaddr_in6);
	}

	else { // IPv4
		struct sockaddr_in server;
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
	}

	// count speed etc:
	const long long int w_len = 1; // CONFIG: show periodial stat each N seconds with average speeds/values from that time window. (window length in seconds)

	long long int count_all=0;  // all since start
	long long int time_all=0; // all time since start
	long long int w_count_b=0;  // in this window: count of bytes
	long long int w_count_pkt=0;  // in this window: count of packets
	long long int w_time1=0; // start when this window started

	w_time1=time(NULL);
	time_all = w_time1;

	while (1) {
		if (mode_ipv6) {
			n = recvfrom(sock,buf,BUFFER_SIZE,0,(struct sockaddr *)&from6,&fromlen);
		}
		else {
			n = recvfrom(sock,buf,BUFFER_SIZE,0,(struct sockaddr *)&from,&fromlen);
		}
		if (n < 0) error("recvfrom");
		w_count_b += n; // number of bytes (received)
		++w_count_pkt;
		++count_all;

		// printf("read=%d ", n);

		if (1==(count_all % 100))	{
			long long int w_time2 = time(NULL); // now


			if (w_time2 >= w_time1 + w_len) {
				printf("\nStats at count: %lld \n", count_all);
				printf("IPv6 mode: UDP %s\n" , (mode_ipv6 ? "YES" : "no (using IPv4. Try option ipv6 to use IPv6 instead)"));
				

				buf[n]='\0'; // TODO secure?
				printf("Received: [%s]\n", buf);


				double all_speed = w_count_b / ( ((double)w_time2) - time_all); // since start

				// in window:
				double speed = w_count_b / ( ((double)w_time2) - w_time1);
				double speed_pkt = w_count_pkt / ( ((double)w_time2) - w_time1);
				double w_avg_pkt_size = w_count_b / ((double)w_count_pkt);
				if (w_count_pkt == 0) w_avg_pkt_size=-1;

				printf("In window:\n");
				printf("speed:       avg pkt size %f bits (%f bytes)\n",  w_avg_pkt_size*8, w_avg_pkt_size);
				printf("speed: %f    pkt  /sec\n", speed_pkt);
				printf("speed: %f    byte /sec\n", speed);
				printf("speed: %f Ki byte /sec\n", speed/1024);
				printf("speed: %f Mi byte /sec\n", speed/(1024*1024));
				printf("speed: %f    bit  /sec\n", 8*speed);
				printf("speed: %f Ki bit  /sec\n", 8*speed/1024);
				printf("speed: %f Mi bit  /sec\n", 8*speed/(1024*1024));
				printf("Since start:\n");
				printf("speed: %f Mi byte /sec\n", all_speed/(1024*1024));
				printf("speed: %f Mi bit  /sec\n", 8*all_speed/(1024*1024));

				// restart window counter:
				w_time1 = w_time2;
				w_count_b = 0;
				w_count_pkt = 0;
			}
		}
	}
	return 0;
 }

