// from https://networking-class.readthedocs.io/en/latest/tutorials/raw_ethernet/raw_ethernet.html

/*
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 * Original version from Austin Martin:
 * https://gist.github.com/austinmarton/1922600
 *
 * Adapted by Paul Craven
 */

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <unistd.h>

/* Replace the bytes below with the MAC address
   you are sending to.
   So if 'ifconfig' on the RECEIVER (not the sender) says:
   wlan0     Link encap:Ethernet  HWaddr b8:27:eb:44:08:62
   You'd replace the bytes below with:
   #define DEST_MAC0 0xB8
   #define DEST_MAC1 0x27
   #define DEST_MAC2 0xEB
   #define DEST_MAC3 0x44
   #define DEST_MAC4 0x08
   #define DEST_MAC5 0x62
*/

// is set to broadcast MAC:
#define DEST_MAC0    0xFF
#define DEST_MAC1    0xFF
#define DEST_MAC2    0xFF
#define DEST_MAC3    0xFF
#define DEST_MAC4    0xFF
#define DEST_MAC5    0xFF

#define DEFAULT_IF  "wlan0"
#define BUF_SIZ     1024*64

int main(int argc, char *argv[])
{
    int sockfd;
    int i;
    struct ifreq if_idx;
    struct ifreq if_mac;
    int tx_len; // real tx_len that we filled in
    char sendbuf[BUF_SIZ];
    struct sockaddr_ll socket_address;
    char ifName[IFNAMSIZ];
    int flood=0; // should we flood at max speed

    /* Get interface name */
    if (argc > 1)
        strcpy(ifName, argv[1]);
    else
        strcpy(ifName, DEFAULT_IF);

    if (argc > 2) {
    	if (0==strcmp("flood",argv[2])) { flood=1; }
    	else {
	    	if (0==strcmp("slow",argv[2])) { flood=0; }
	    	else {
	    		printf("Use 'flood' or 'slow' as 2nd argument.\n");
	    		return 1;
				}
	    }
		}

		int tx_want_len=-1; // what size we want to send. -1 means to send that many bytes as we construct e.g. tx_len

		if (argc > 3) {
			tx_want_len = atoi(argv[3]);
		}

		if (tx_want_len>=0) {
			if (tx_want_len >= BUF_SIZ) {
				printf("Too large data (increase the buffer in source code)\n");
				return -2;
			}
		}

    /* Open RAW socket to send on */
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
        perror("socket");
    }

    /* Get the index of the interface to send on */
    memset(&if_idx, 0, sizeof(struct ifreq));
    strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
    if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
        perror("SIOCGIFINDEX");

    /* Get the MAC address of the interface to send on */
    memset(&if_mac, 0, sizeof(struct ifreq));
    strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
    if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
        perror("SIOCGIFHWADDR");


		/* Buffer of BUF_SIZ bytes we'll construct our frame in.
			 First, clear it all to zero. */
		memset(sendbuf, 0, BUF_SIZ);

    // Loop forever
    while(1) {


        tx_len = 0;

        /* Construct the Ethernet header */

        /* Ethernet header */
        /* Destination address */
        sendbuf[tx_len++] = DEST_MAC0;
        sendbuf[tx_len++] = DEST_MAC1;
        sendbuf[tx_len++] = DEST_MAC2;
        sendbuf[tx_len++] = DEST_MAC3;
        sendbuf[tx_len++] = DEST_MAC4;
        sendbuf[tx_len++] = DEST_MAC5;

        /* Create the source */
        sendbuf[tx_len++] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
        sendbuf[tx_len++] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
        sendbuf[tx_len++] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
        sendbuf[tx_len++] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
        sendbuf[tx_len++] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
        sendbuf[tx_len++] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];

        /* Ethertype field */
        sendbuf[tx_len++] = 0xfc;
        sendbuf[tx_len++] = 0x40;

        /*
        Packet data
        This is the 'payload'. Replace this with your real data.
        Because you'll probably have more interesting things to send
        than the hex 0xDEAD 0xBEEF
        */
        sendbuf[tx_len++] = 0xde;
        sendbuf[tx_len++] = 0xad;
        sendbuf[tx_len++] = 0xbe;
        sendbuf[tx_len++] = 0xef;

        /* Index of the network device */
        socket_address.sll_ifindex = if_idx.ifr_ifindex;
        /* Address length*/
        socket_address.sll_halen = ETH_ALEN;
        /* Destination MAC */
        socket_address.sll_addr[0] = DEST_MAC0;
        socket_address.sll_addr[1] = DEST_MAC1;
        socket_address.sll_addr[2] = DEST_MAC2;
        socket_address.sll_addr[3] = DEST_MAC3;
        socket_address.sll_addr[4] = DEST_MAC4;
        socket_address.sll_addr[5] = DEST_MAC5;

				int tx_send = tx_len; // how much data actually to send. start with the constructed bytes
        if (tx_want_len >=0 ) tx_send = tx_want_len;

        /* Send packet */
        if (sendto(sockfd, sendbuf, tx_send, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
				{
            printf("Send failed\n");
            return 3;
        }
        else {
        		if (!flood) {
							printf("Sent :");
							for (i=0; i < tx_len; i++)
									printf("%02x:", sendbuf[i]);
							printf("\n");
						}
        }

        if (!flood) {
					/* Wait specified number of microseconds
						 */
					usleep(1000000 / 10);
				}
    }
    return 0;
}
