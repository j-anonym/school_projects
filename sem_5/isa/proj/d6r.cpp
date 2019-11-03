//Author: Ján Vavro
//Login: xvavro05
//ISA-dhcp-relay

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <err.h>
#include <iostream>
#include <netinet/ether.h>
#include <ctime>
#include <pcap/pcap.h>
#include <linux/ipv6.h>
#include <cstring>
#include <unistd.h>


#define ERR_BUFF_SIZE 256

#define ETHERNET_HDRLEN (14)    // Ethernet header length
#define IP6_HDRLEN (40)         // IPv6 header length
#define UDP_HDRLEN  (8)         // UDP header length, excludes data


#define CAPTURE_FILTER "port 547"
typedef struct in_addr in_addr;

void mypcap_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);

int n = 0;

int main(int argc, char *argv[]) {
    char errbuff[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    struct bpf_program fp;          // the compiled filter

    //TODO args
    int i = 0;
    pcap_if_t *interfaces, *temp, *int_to_be_sniffed = NULL;
    if (pcap_findalldevs(&interfaces, errbuff) == -1) {
        err(1, "pcap_findalldevs() failed");
    }

    printf("The interfaces present on the system are:\n");
    for (temp = interfaces; temp; temp = temp->next) {
        printf("%d  :  %s\n", i++, temp->name);
        if (strcmp("vboxnet0", temp->name) == 0) {
            int_to_be_sniffed = temp;
        }
    }

    if (!int_to_be_sniffed) {
        err(1, "cant open interface for sniffing\n");
    }

    for (temp = interfaces; temp != NULL; temp = temp->next)
    {

        pcap_addr_t *dev_addr; //interface address that used by pcap_findalldevs()

        /* check if the device captureble*/
        for (dev_addr = temp->addresses; dev_addr != NULL; dev_addr = dev_addr->next) {
            if (dev_addr->addr->sa_family == AF_INET6 && dev_addr->addr && dev_addr->netmask) {
                char ip6[INET6_ADDRSTRLEN];
                char ip6_netmask[INET6_ADDRSTRLEN];

                inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)dev_addr->addr)->sin6_addr), ip6, INET6_ADDRSTRLEN);
                inet_ntop(AF_INET6, (struct sockaddr_in6 *)dev_addr->netmask, ip6_netmask, INET6_ADDRSTRLEN);

                printf("Found a device %-10s on address %s with netmask %s\n", temp->name, ip6, ip6_netmask);
            }
        }
    }


    // open the interface for live sniffing
    if ((handle = pcap_open_live(int_to_be_sniffed->name, BUFSIZ, 1, 1000, errbuff)) == NULL)
        err(1, "pcap_open_live() failed");

    // compile the filter
    if (pcap_compile(handle, &fp, CAPTURE_FILTER, 0, 0) == -1)
        err(1, "pcap_compile() failed");

    // set the filter to the packet capture handle
    if (pcap_setfilter(handle, &fp) == -1)
        err(1, "pcap_setfilter() failed");

    // read packets from the interface in the infinite loop (count == -1)
    // incoming packets are processed by function mypcap_handler()
    if (pcap_loop(handle, -1, mypcap_handler, NULL) == -1)
        err(1, "pcap_loop() failed");

    // close the capture device and deallocate resources
    pcap_close(handle);
    return 0;
}

void mypcap_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
    struct ip *my_ip;               // pointer to the beginning of IP header
    struct ipv6hdr *ip6;
    struct ether_header *eptr;      // pointer to the beginning of Ethernet header
    const struct tcphdr *my_tcp;    // pointer to the beginning of TCP header
    const struct udphdr *my_udp;    // pointer to the beginning of UDP header
    u_int size_ip;

    n++;
    // print the packet header data
    printf("Packet no. %d:\n", n);
    printf("\tLength %d, received at %s", header->len, ctime((const time_t *) &header->ts.tv_sec));

    // read the Ethernet header
    eptr = (struct ether_header *) packet;
    printf("\tSource MAC: %s\n", ether_ntoa((const struct ether_addr *) &eptr->ether_shost));
    printf("\tDestination MAC: %s\n", ether_ntoa((const struct ether_addr *) &eptr->ether_dhost));

    if (ntohs(eptr->ether_type) == ETHERTYPE_IPV6) {               // see /usr/include/net/ethernet.h for types
        printf("\tEthernet type is 0x%x, i.e., IPv6 packet\n", ntohs(eptr->ether_type));
        ip6 = (struct ipv6hdr *) (packet + ETHERNET_HDRLEN);
        char ip6_saddr[INET6_ADDRSTRLEN];
        memset(ip6_saddr, 0, sizeof(ip6_saddr));
        inet_ntop(AF_INET6, &ip6->saddr, ip6_saddr, sizeof(ip6_saddr));
        printf("From: %s\n", ip6_saddr);
    }


    int sock;
    socklen_t clilen;
    struct sockaddr_in6 server_addr, client_addr;
    char buffer[1024];
    char addrbuf[INET6_ADDRSTRLEN];
    //create socket for communicating with dhcp server
    ;

    if ((sock = socket(PF_INET6, SOCK_DGRAM, 0)) < 0) {
        err(1, "creating socket");
    }

#define PORT 5477
#define MESSAGE "hi there"
#define SERVADDR "2001:67c:1220:80c::93e5:dd2" //TODO argument

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, SERVADDR, &server_addr.sin6_addr);

    server_addr.sin6_port = htons(PORT);

    /* now send a datagram */
    if (sendto(sock, MESSAGE, sizeof(MESSAGE), 0,
               (struct sockaddr *) &server_addr,
               sizeof(server_addr)) < 0) {
        err(1, "sendto failed");
    }

    printf("waiting for a reply...\n");
    clilen = sizeof(client_addr);
    if (recvfrom(sock, buffer, 1024, 0,
                 (struct sockaddr *) &client_addr,
                 &clilen) < 0) {
        err(1, "recvfrom failed");
    }

    printf("got '%s' from %s\n", buffer,
           inet_ntop(AF_INET6, &client_addr.sin6_addr, addrbuf,
                     INET6_ADDRSTRLEN));

    close(sock);
}