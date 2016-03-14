/*
 * tunproxy.c --- small demo program for tunneling over UDP with tun/tap
 *
 * Copyright (C) 2003  Philippe Biondi <phil@secdev.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */



#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <error.h>

#include <sodium.h>

#include "NetPlatform.h"

#define PERROR(x) do { perror(x); exit(1); } while (0)
#define ERROR(x, args ...) do { fprintf(stderr,"ERROR:" x, ## args); exit(1); } while (0)
#define BUFF_SIZE 1500
#define ADDITIONAL_DATA (const unsigned char *) "123456"
#define ADDITIONAL_DATA_LEN 6

char MAGIC_WORD[] = "Wazaaaaaaaaaaahhhh !";


void usage()
{
	// TODO nice and correct help see below
	fprintf(stderr, "Usage: tunproxy [-s port|-c targetip:port] [-e] [-I] [-O] [-a] [-F ip_fill_char]\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	if(sodium_init() == -1) {
        return 1;
    }
	struct sockaddr_in sin, sout, from;
	struct ifreq ifr;
	int fd, s, port, PORT, l;
	unsigned int soutlen, fromlen;
	char c, *p, *ip=NULL;
	char buf[BUFF_SIZE];
	fd_set fdset;

	uint8_t ip_fill_char = 0;

	int MODE = 0, TUNMODE = IFF_TUN, DEBUG = 0;

	int drop_outgoing=0, drop_incoming=0; // debug: should we drop data that is outgoing (or ingoing)

	int warned_drop_outgoing=0, warned_drop_incoming=0; // warnings for user, did we warned him that we are dropping outgoing (or incoming)
	int use_auth=1; // should we use authorization on start

	while ((c = getopt(argc, argv, "s:c:ehdIOAp:F:")) != -1) {
		switch (c) {
		case 'h': usage(); break;
		case 'd':	DEBUG++; break;
		case 's':
			MODE = 1;
			PORT = atoi(optarg);
			break;
		case 'c':
			MODE = 2;
			p = (char*)memchr(optarg,':',16);
			if (!p) ERROR("invalid argument : [%s]\n",optarg);
			*p = 0;
			ip = optarg;
			port = atoi(p+1);
			PORT = port;
			break;
		case 'p': // add a peer / set a peer
			p = (char*)memchr(optarg,':',16);
			if (!p) ERROR("invalid argument : [%s]\n",optarg);
			*p = 0;
			ip = optarg;
			port = atoi(p+1);
			break;
		case 'e':
			TUNMODE = IFF_TAP;
			break;
		case 'I':	drop_incoming=1; break;
		case 'O':	drop_outgoing=1; break;
		case 'A':	use_auth=0; break;
		case 'F':	ip_fill_char=optarg[0]; break;
		default:
			usage();
		}
	}
	printf("DEBUG=%d\n", DEBUG);

	if (MODE == 0) usage();


	if ( (fd = open("/dev/net/tun",O_RDWR)) < 0) PERROR("open");

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = TUNMODE || IFF_MULTI_QUEUE;
	strncpy(ifr.ifr_name, "toto%d", IFNAMSIZ);
	if (ioctl(fd, TUNSETIFF, (void *)&ifr) < 0) PERROR("ioctl");

	printf("Allocated interface %s. Configure and use it\n", ifr.ifr_name);

	printf("set iface address for iface %s\n", ifr.ifr_name);
	uint8_t address[16];
	int i;
	for (i=0; i<16; ++i) address[i] = ip_fill_char;

	address[0] = 0xFD;
	address[1] = 0x00;
	NetPlatform_addAddress(ifr.ifr_name, address, 8, Sockaddr_AF_INET6);
	
	/* the old code to open UDP socket:
	s = socket(AF_INET, SOCK_DGRAM, 0);
  bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY; // or  htonl(INADDR_ANY); TODO ?!
	sin.sin_port = htons(PORT);
	if ( bind(s,(struct sockaddr *)&sin, sizeof(sin)) < 0) PERROR("bind");
	*/

	printf("We will listen for peers on udp port PORT=%d \n", PORT);

	// struct sockaddr_in sin;
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) error(1, 1, "Opening socket");
	//length = sizeof(server);
	bzero(&sin, sizeof(sin));
	sin.sin_family=AF_INET;
	sin.sin_addr.s_addr=INADDR_ANY;
	sin.sin_port=htons(PORT); 
	// if (bind(sock,(struct sockaddr *)&server,length)<0)
	printf("Doing the bind, on address %d on port %d\n", sin.sin_addr.s_addr, ntohs(sin.sin_port));
	if (bind(s,(struct sockaddr *)&sin, sizeof(sin))<0) error(1, 1, "binding"); // <--- bind
	printf("Doing the bind - DONE\n");
	// fromlen = sizeof(struct sockaddr_in);

	printf("After bind in line %d\n", __LINE__);

	fromlen = sizeof(from);

	if (use_auth) { 
		printf("Doing authorization\n");

	if (MODE == 1) {
		printf("Will wait for the passwor packet now...%d\n", __LINE__);
		while(1) {
			printf("Trying to receive password...%d\n", __LINE__);
			l = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&from, &fromlen);
			printf("Read password packet len=%d in line %d\n", l, __LINE__);
			if (l < 0) PERROR("recvfrom");
			if (strncmp(MAGIC_WORD, buf, sizeof(MAGIC_WORD)) == 0)
				break;
			printf("Bad magic word from %s:%i\n", 
			       inet_ntoa(from.sin_addr), ntohs(from.sin_port));
		} 
		printf("Got correct password in line %d\n", __LINE__);
		printf("Sending reply line %d\n", __LINE__);
		l = sendto(s, MAGIC_WORD, sizeof(MAGIC_WORD), 0, (struct sockaddr *)&from, fromlen);
		printf("Sent reply line %d\n", __LINE__);
		if (l < 0) PERROR("sendto");
	} else {
		printf("Will send the password now...\n");
		from.sin_family = AF_INET;
		from.sin_port = htons(port);
		inet_aton(ip, &from.sin_addr);
		l =sendto(s, MAGIC_WORD, sizeof(MAGIC_WORD), 0, (struct sockaddr *)&from, sizeof(from));
		if (l < 0) PERROR("sendto");
		l = recvfrom(s,buf, sizeof(buf), 0, (struct sockaddr *)&from, &fromlen);
		if (l < 0) PERROR("recvfrom");
		if (strncmp(MAGIC_WORD, buf, sizeof(MAGIC_WORD) != 0))
			ERROR("Bad magic word for peer\n");
	}

	printf("Auth done - connection with %s:%i established\n",
	       inet_ntoa(from.sin_addr), ntohs(from.sin_port));

	} // use_auth
	else {
		printf("Skipped auth - setting the peer socket (from) to addres of peer.\n");
		if (!ip) ERROR("You must set the peer address with proper option.\n");
		printf("Peer address is set to ip=%s and port=%d\n", ip, port);
		from.sin_family = AF_INET;
		from.sin_port = htons(port);
		inet_aton(ip, &from.sin_addr);
		printf("Prepared connection address for our peer.\n");
		// l = sendto(s, MAGIC_WORD, sizeof(MAGIC_WORD), 0, (struct sockaddr *)&from, sizeof(from));
	}


	// sodium
	unsigned char nonce[crypto_aead_chacha20poly1305_NPUBBYTES];
	unsigned char key[crypto_aead_chacha20poly1305_KEYBYTES];
	memset(nonce, 'a', crypto_aead_chacha20poly1305_NPUBBYTES);
	memset(key, 'b', crypto_aead_chacha20poly1305_NPUBBYTES);
	unsigned char ciphertext[BUFF_SIZE + crypto_aead_chacha20poly1305_ABYTES];
	//unsigned char ciphertext[BUFF_SIZE];
	unsigned long long ciphertext_len = 0;
	unsigned char decrypted[BUFF_SIZE];
	unsigned long long decrypted_len;

	printf("Main loop\n");

	while (1) {
		FD_ZERO(&fdset); // wait for:
		FD_SET(fd, &fdset); // for evet on TUN
		FD_SET(s, &fdset); // for event on peering socket
		if (select(fd+s+1, &fdset,NULL,NULL,NULL) < 0) PERROR("select"); // waiting here

		if (FD_ISSET(fd, &fdset)) { // data incoming on TUN (we should send it out)
			if (DEBUG) write(1,">", 1);
			l = read(fd, buf, sizeof(buf)); // read data from TUN
			if (l < 0) PERROR("read");
			if (!drop_outgoing) {
				crypto_aead_chacha20poly1305_encrypt(ciphertext, &ciphertext_len,
                                         buf, BUFF_SIZE,
                                         ADDITIONAL_DATA, ADDITIONAL_DATA_LEN,
                                         NULL, nonce, key);
				if (sendto(s, ciphertext, ciphertext_len, 0, (struct sockaddr *)&from, fromlen) < 0) PERROR("sendto");
			} else {
				if (!warned_drop_outgoing) { 
					printf("Warning: we are dropping outgoing packets, droping one now (will not warn again about this)\n"); 
					warned_drop_outgoing=1;
				}
			}
		} 
		else { // data incoming from peering (we should input it into the TUN to our localhost) -or- route it further in mesh
			if (DEBUG) write(1,"<", 1);
			l = recvfrom(s, ciphertext, sizeof(ciphertext), 0, (struct sockaddr *)&sout, &soutlen);
			if (crypto_aead_chacha20poly1305_decrypt(decrypted, &decrypted_len,
                                             NULL,
                                             ciphertext, ciphertext_len,
                                             ADDITIONAL_DATA,
                                             ADDITIONAL_DATA_LEN,
                                             nonce, key)) {
				printf("decrypt fail: message forged!\n");
				continue;
			}
			else {
				printf("decrypt ok\n");
			}

			/*if ((sout.sin_addr.s_addr != from.sin_addr.s_addr) || (sout.sin_port != from.sin_port))
				printf("Got packet from  %s:%i instead of %s:%i\n", 
				       inet_ntoa(sout.sin_addr), ntohs(sout.sin_port),
				       inet_ntoa(from.sin_addr), ntohs(from.sin_port));
				*/

			if (!drop_incoming) {

				if (write(fd, (char*)decrypted, decrypted_len) < 0) PERROR("write");
			} else {
				if (!warned_drop_incoming) { 
					printf("Warning: we are dropping incoming packets, droping one now (will not warn again about this)\n"); 
					warned_drop_incoming=1;
				}
			}
		}
	}
}
	       
	
