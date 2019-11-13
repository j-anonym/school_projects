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
#include <net/if.h>
#include <netinet/if_ether.h>
#include <err.h>
#include <netinet/ether.h>
#include <ctime>
#include <pcap/pcap.h>
#include <linux/ipv6.h>
#include <cstring>
#include <unistd.h>
#include <thread>
#include <netdb.h>

using namespace std;

#define ERR_BUFF_SIZE 256

#define ETHERNET_HDRLEN (14)    // Ethernet header length
#define IP6_HDRLEN (40)         // IPv6 header length
#define UDP_HDRLEN  (8)         // UDP header length, excludes data
#define RELAY_F_WITHOUT_OPTIONS_LEN (34)

#define CAPTURE_FILTER "udp port 547"
#define OPTION_RELAY_MSG (9) //see RFC 8415
#define OPTION_CLIENT_LINKLAYER_ADDR (79) //see RFC 6939

#define BUFF_SIZE (1024)
#define DHCPV6_MULTICAST_CLIENT "ff02::1:2"

typedef struct relay_forward_message {
    uint8_t msg_type;
    uint8_t hop_count;
    uint8_t link_addr[16];
    uint8_t peer_addr[16];
    uint8_t options[];
} t_relay_forward_message;

typedef struct relay_option_relay_message {
    uint16_t option_code;
    uint16_t length;
    uint8_t option_data[];
} t_relay_option_relay_message;

typedef struct relay_option_ll_address {
    uint16_t option_code;
    uint16_t length;
    uint16_t link_layer_type;
    uint8_t mac_addr[ETH_ALEN];
} t_relay_option_ll_address;

void mypcap_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);

void sniff_for_client(pcap_t *handle);

int n = 0;
pcap_if_t *int_to_be_sniffed = NULL;
struct in6_addr ip6_link_address;


int main(int argc, char *argv[]) {
    char errbuff[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    pcap_t *handle_serv;
    struct bpf_program fp;          // the compiled filter


    //TODO args
    int i = 0;
    pcap_if_t *interfaces, *temp, *serv_if = NULL;
    if (pcap_findalldevs(&interfaces, errbuff) == -1) {
        err(1, "pcap_findalldevs() failed");
    }

    for (temp = interfaces; temp != NULL; temp = temp->next) {

        pcap_addr_t *dev_addr; //addresses from pcap_findalldevs

        //check for capturable inferfaces with IPv6
        for (dev_addr = temp->addresses; dev_addr != NULL; dev_addr = dev_addr->next) {
            if (dev_addr->addr->sa_family == AF_INET6 && dev_addr->addr && dev_addr->netmask) {
                char ip6[INET6_ADDRSTRLEN];
                char ip6_netmask[INET6_ADDRSTRLEN];

                inet_ntop(AF_INET6, &(((struct sockaddr_in6 *) dev_addr->addr)->sin6_addr), ip6, INET6_ADDRSTRLEN);
                inet_ntop(AF_INET6, (struct sockaddr_in6 *) dev_addr->netmask, ip6_netmask, INET6_ADDRSTRLEN);
                printf("Found a device %-10s on address %s with netmask %s\n", temp->name, ip6, ip6_netmask);

                //TODO
                if (strcmp("vboxnet0", temp->name) == 0 && int_to_be_sniffed == NULL) {
                    int_to_be_sniffed = temp;
                    ip6_link_address = ((struct sockaddr_in6 *) dev_addr->addr)->sin6_addr;
                } else if (strcmp("enp3s0", temp->name) == 0 && serv_if == NULL) {
                    serv_if = temp;
                }
            }
        }

    }

    if (!int_to_be_sniffed) {
        err(1, "Can't find device for sniffing");
    }

    if (!serv_if)
        err(1, "Can't find device for communicating with server");


    if ((handle = pcap_open_live(int_to_be_sniffed->name, BUFSIZ, 1, 1000, errbuff)) == NULL)
        err(1, "pcap_open_live() failed");

    if (pcap_compile(handle, &fp, CAPTURE_FILTER, 0, 0) == -1)
        err(1, "pcap_compile() failed");

    if (pcap_setfilter(handle, &fp) == -1)
        err(1, "pcap_setfilter() failed");

    thread th1(sniff_for_client, handle);
    printf("on si chyta a ja som tu\n");
    sleep(10);



    // close the capture device and deallocate resources
    th1.join();
    pcap_close(handle);
    return 0;
}

void sniff_for_client(pcap_t *handle) {
    if (pcap_loop(handle, -1, mypcap_handler, NULL) == -1)
        err(1, "pcap_loop() failed");
}

void mypcap_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
    struct ipv6hdr *ip6_h;
    struct ether_header *eth_h;      // pointer to the beginning of Ethernet header
    struct udphdr *udp_h;

    // read the Ethernet header
    eth_h = (struct ether_header *) packet;
    printf("\tSource MAC: %s\n", ether_ntoa((const struct ether_addr *) &eth_h->ether_shost));
    printf("\tDestination MAC: %s\n", ether_ntoa((const struct ether_addr *) &eth_h->ether_dhost));

    if (ntohs(eth_h->ether_type) != ETHERTYPE_IPV6) {               // ethernet.h
        return;
    }

    printf("\tEthernet type is 0x%x, i.e., IPv6 packet\n", ntohs(eth_h->ether_type));
    ip6_h = (struct ipv6hdr *) (packet + ETHERNET_HDRLEN);
    char ip6_saddr[INET6_ADDRSTRLEN];
    memset(ip6_saddr, 0, sizeof(ip6_saddr));
    inet_ntop(AF_INET6, &ip6_h->saddr, ip6_saddr, sizeof(ip6_saddr));
    printf("From: %s\n", ip6_saddr);


    //if (ip6_h->nexthdr == 17) { //UDP check now in capture filer
    udp_h = (struct udphdr *) (packet + ETHERNET_HDRLEN + IP6_HDRLEN); // pointer to the UDP header
    printf("\tSrc port = %d, dst port = %d, length %d\n", ntohs(udp_h->uh_sport), ntohs(udp_h->uh_dport),
           ntohs(udp_h->uh_ulen));

/* //print udp content
    unsigned char data[ntohs(udp_h->uh_ulen) - UDP_HDRLEN];
    memcpy(data, (char *)udp_h + UDP_HDRLEN, ntohs(udp_h->uh_ulen) - UDP_HDRLEN);
    for (unsigned int i = 0; i < ntohs(udp_h->uh_ulen) - UDP_HDRLEN; ++i) {
        printf("%02x\n", data[i]);
    }*/

    //creating message for server
    int forward_message_len =
            sizeof(t_relay_forward_message) + sizeof(t_relay_option_relay_message) + ntohs(udp_h->uh_ulen) -
            UDP_HDRLEN +
            sizeof(t_relay_option_ll_address);
    t_relay_forward_message *forward_message = (t_relay_forward_message *) malloc(forward_message_len
    );

    if (forward_message == NULL) {
        err(1, "Cannot allocate memory");
    }
    forward_message->hop_count = 0;
    forward_message->msg_type = 12;
    memcpy(forward_message->peer_addr, &ip6_h->saddr, 16);
    memcpy(forward_message->link_addr, &ip6_link_address, 16);

    t_relay_option_relay_message *o_relay_message = (t_relay_option_relay_message *) ((char *) forward_message +
                                                                                      sizeof(t_relay_forward_message));
    //fill relay message
    o_relay_message->option_code = htons(OPTION_RELAY_MSG);
    o_relay_message->length = htons(ntohs(udp_h->uh_ulen) - UDP_HDRLEN);
    memcpy(o_relay_message->option_data, (char *) udp_h + UDP_HDRLEN, ntohs(udp_h->uh_ulen) - UDP_HDRLEN);

    t_relay_option_ll_address *o_link_layer = (t_relay_option_ll_address *) ((char *) o_relay_message +
                                                                             (sizeof(t_relay_option_relay_message) +
                                                                              ntohs(o_relay_message->length)));
    //fill MAC address
    o_link_layer->option_code = htons(OPTION_CLIENT_LINKLAYER_ADDR);
    o_link_layer->length = htons(8);
    o_link_layer->link_layer_type = htons(1);
    memcpy(o_link_layer->mac_addr, eth_h->ether_shost, ETH_ALEN);


    int sock_send_to_server, sock_recv_from_server;
    struct sockaddr_in6 server;
    //create socket for communicating with dhcp server

    if ((sock_send_to_server = socket(PF_INET6, SOCK_DGRAM, 0)) < 0) {
        err(1, "can't create socket");
    }

    if ((sock_recv_from_server = socket(PF_INET6, SOCK_RAW, IPPROTO_UDP)) < 0) {
        err(1, "can't create socket");
    }

#define PORT (547)
#define PORT_CLIENT (546)
#define SERVADDR "2001:67c:1220:80c::93e5:dd2" //TODO argument

    memset(&server, 0, sizeof(server));
    server.sin6_family = AF_INET6;
    inet_pton(AF_INET6, SERVADDR, &server.sin6_addr);
    socklen_t server_length = sizeof(server);

    server.sin6_port = htons(PORT);


    char buffer[BUFF_SIZE] = {0,};
    int read_count;

    // send relay_forward
    if (sendto(sock_send_to_server, forward_message, forward_message_len, 0, (struct sockaddr *) &server,
               server_length) < 0) {
        err(1, "sento()");
    }

    // recv relay_reply from server
    if ((read_count = recvfrom(sock_recv_from_server, (char *) buffer, sizeof(buffer), 0, (struct sockaddr *) &server,
                               &server_length) < 0)) {
        err(1, "rcvfrom()");
    }

    /* print content of response
    for (int i = 0; i < BUFF_SIZE; ++i) {
        printf("%02x",buffer[i]);
    }
     */


    t_relay_forward_message *mess_relay_reply = (t_relay_forward_message *) (buffer + UDP_HDRLEN);
    if (mess_relay_reply->msg_type != 13) {
        err(1, "wrong response from server");
    }
    t_relay_option_relay_message *relay_message = (t_relay_option_relay_message *) (buffer + UDP_HDRLEN +
                                                                                    sizeof(t_relay_forward_message));
    int fd;
    if ((fd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
        err(1, "socket()");

    struct sockaddr_in6 address;
    memset(&address, 0, sizeof(address));
    address.sin6_family = AF_INET6;
    address.sin6_port = htons(PORT_CLIENT);


    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "vboxnet0");
    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
        err(1,"setsockopt()");
    }

    memcpy(&address.sin6_addr, &mess_relay_reply->peer_addr, 16);


    memset(ip6_saddr, 0, sizeof(ip6_saddr));
    inet_ntop(AF_INET6, &address.sin6_addr, ip6_saddr, sizeof(ip6_saddr));
    printf("To: %s\n", ip6_saddr);

    if ((n = sendto(fd, buffer + UDP_HDRLEN + sizeof(t_relay_forward_message) + sizeof(t_relay_option_relay_message),
                    ntohs(relay_message->length), 0, (struct sockaddr *) &address, sizeof(address))) < 0)
        err(1, "sendto()");
    printf("%d\n", n);


}