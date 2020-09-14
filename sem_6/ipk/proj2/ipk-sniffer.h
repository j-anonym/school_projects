#ifndef PROJ2_IPK_SNIFFER_H
#define PROJ2_IPK_SNIFFER_H

#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <err.h>
#include <netinet/ether.h>
#include <pcap/pcap.h>
#include <cstring>
#include <unistd.h>
#include <syslog.h>
#include <thread>
#include <vector>
#include <map>
#include <iostream>
#include <getopt.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>



#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <err.h>




struct arguments {
    char *interface;
    int port;
    bool isTcp;
    bool isUdp;
    int num;
} arguments_default = {NULL, -1, false, false, -1};
typedef struct arguments t_arguments;


//HEADER FROM linux/ipv6.h
struct ipv6hdr {
    __u8	priority:4,
            version:4;
    __u8	flow_lbl[3];
    __be16			payload_len;
    __u8			nexthdr;
    __u8			hop_limit;

    struct	in6_addr	saddr;
    struct	in6_addr	daddr;
};

#endif //PROJ2_IPK_SNIFFER_H
