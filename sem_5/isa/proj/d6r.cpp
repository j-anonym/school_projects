//Author: Ján Vavro
//Login: xvavro05
//ISA-dhcp-relay

#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <err.h>
#include <netinet/ether.h>
#include <pcap/pcap.h>
#include <linux/ipv6.h>
#include <cstring>
#include <unistd.h>
#include <syslog.h>
#include <thread>
#include <vector>
#include <map>
#include <iostream>
#include <getopt.h>

using namespace std;

#define ETHERNET_HDRLEN (14)    // Ethernet header length
#define IP6_HDRLEN (40)         // IPv6 header length
#define UDP_HDRLEN  (8)         // UDP header length, excludes data

#define CAPTURE_FILTER "udp port 547"
#define OPTION_RELAY_MSG (9) //see RFC 8415
#define OPTION_INTERFACE_ID (18) //see RFC 8415
#define OPTION_CLIENT_LINKLAYER_ADDR (79) //see RFC 6939

#define BUFF_SIZE (1024)

#define PORT (547)
#define PORT_CLIENT (546)

map<string, string> mac_addr_map;

typedef struct arguments {
    char *server_arg;
    char *interface;
    bool log_enabled;
    bool debug_enabled;
} t_arguments;

t_arguments args_cli;

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

void sniff_for_client(pcap_if_t *int_to_be_sniffed);

struct udphdr *get_udph_from_ipv6h(struct ipv6hdr *);

void process_msg_from_server(const char *buffer, int read_count);

void server_service(struct sockaddr_in6 server);

void print_usage() {
    printf("USAGE:# ./d6r -s server [-l] [-d] [-i interface]\n");
}

int main(int argc, char *argv[]) {
    int opt;
    args_cli.interface = NULL;
    args_cli.server_arg = NULL;
    args_cli.debug_enabled = false;
    args_cli.log_enabled = false;

    while ((opt = getopt(argc, argv, ":s:ldi:")) != -1) {
        switch (opt) {
            case 's':
                args_cli.server_arg = optarg;
                break;
            case 'l':
                args_cli.log_enabled = true;
                break;
            case 'd':
                args_cli.debug_enabled = true;
                break;
            case 'i':
                args_cli.interface = optarg;
                break;
            case ':':
                fprintf(stderr, "ERROR option '-s' needs a value\n");
                print_usage();
                exit(1);
                break;
            case '?':
                fprintf(stderr, "unknown option: %c\n", optopt);
                print_usage();
                exit(1);
                break;
        }
    }

    // optind is for the extra arguments that are not parsed
    if (optind < argc) {
        fprintf(stderr, "ERROR unrecognized extra argument: %s\n", argv[optind]);
        print_usage();
        exit(1);
    }

    if (!args_cli.server_arg) {
        fprintf(stderr, "ERROR: argument with server ipv6 address '-s' is compulsory");
        print_usage();
        exit(1);
    }


    char errbuff[PCAP_ERRBUF_SIZE];

    struct sockaddr_in6 server = {.sin6_family=AF_INET6, .sin6_port=htons(PORT)};
    if (inet_pton(AF_INET6, args_cli.server_arg, &server.sin6_addr) == 0) {
        fprintf(stderr, "ERROR: Wrong IPv6 of server (-s)\n");
        print_usage();
        exit(1);
    }

    openlog("ISA-DHCP-RELAY-LOG", LOG_NDELAY, LOG_DAEMON);

    pcap_if_t *interfaces, *temp;
    if (pcap_findalldevs(&interfaces, errbuff) == -1) {
        err(1, "pcap_findalldevs() failed");
    }

    vector<pcap_if_t *> interfaces_to_be_sniffed;

    for (temp = interfaces; temp != NULL; temp = temp->next) {
        pcap_addr_t *dev_addr; //addresses from pcap_findalldevs

        //check for capturable inferfaces with IPv6
        for (dev_addr = temp->addresses; dev_addr != NULL; dev_addr = dev_addr->next) {
            if (dev_addr->addr->sa_family == AF_INET6 && dev_addr->addr && dev_addr->netmask) {

                if (interfaces_to_be_sniffed.empty() || strcmp(temp->name, interfaces_to_be_sniffed.back()->name) != 0)
                    if (!args_cli.interface || strcmp(args_cli.interface, temp->name) == 0)
                        interfaces_to_be_sniffed.push_back(temp);
            }
        }

    }

    if (interfaces_to_be_sniffed.empty()) {
        if (args_cli.interface) {
            fprintf(stderr, "ERROR interface doesn't exist or doesn't have IPv6 address : %s\n", args_cli.interface);
            print_usage();
            exit(1);
        } else {
            fprintf(stderr, "ERROR cant find interface with IPv6 address\n");
            exit(2);
        }
    }

    thread threads[interfaces_to_be_sniffed.size()];
    int i = 0;
    for (pcap_if_t *interface : interfaces_to_be_sniffed) {
        threads[i] = thread(sniff_for_client, interface);
        i++;
    }

    thread th_server(server_service, server);

    th_server.join();
    closelog();
    return 0;
}

void sniff_for_client(pcap_if_t *int_to_be_sniffed) {
    pcap_t *handle;
    struct bpf_program fp;
    char errbuff[PCAP_ERRBUF_SIZE];

    if (!int_to_be_sniffed) {
        err(1, "Can't find device for sniffing");
    }

    if ((handle = pcap_open_live(int_to_be_sniffed->name, BUFSIZ, 1, 1000, errbuff)) == NULL)
        err(1, "pcap_open_live() failed");

    if (pcap_compile(handle, &fp, CAPTURE_FILTER, 0, 0) == -1)
        err(1, "pcap_compile() failed");

    if (pcap_setfilter(handle, &fp) == -1)
        err(1, "pcap_setfilter() failed");

    if (pcap_loop(handle, -1, mypcap_handler, (u_char *) int_to_be_sniffed) == -1)
        err(1, "pcap_loop() failed");

    pcap_close(handle);

}

void mypcap_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
    struct ipv6hdr *ip6_h;
    struct ether_header *eth_h;
    struct udphdr *udp_h;

    pcap_addr_t *dev_addr;
    struct in6_addr ip6_link_address;
    char *interface_name = ((pcap_if_t *) args)->name;

    for (dev_addr = ((pcap_if_t *) args)->addresses; dev_addr != NULL; dev_addr = dev_addr->next) {
        if (dev_addr->addr->sa_family == AF_INET6 && dev_addr->addr && dev_addr->netmask) {
            ip6_link_address = ((struct sockaddr_in6 *) dev_addr->addr)->sin6_addr;
            break;
        }
    }

    eth_h = (struct ether_header *) packet;

    if (ntohs(eth_h->ether_type) != ETHERTYPE_IPV6) {
        return;
    }

    ip6_h = (struct ipv6hdr *) (packet + ETHERNET_HDRLEN);
    char ip6_string[INET6_ADDRSTRLEN];
    memset(ip6_string, 0, sizeof(ip6_string));
    inet_ntop(AF_INET6, &ip6_h->saddr, ip6_string, sizeof(ip6_string));

    udp_h = (struct udphdr *) get_udph_from_ipv6h(ip6_h); // pointer to the UDP header


    //check message types that should be relayed
    uint8_t msg_type;
    memcpy(&msg_type, (char *) udp_h + UDP_HDRLEN, 1);
    if (msg_type != 1 && msg_type != 3 && msg_type != 4 && msg_type != 5 && msg_type != 6 && msg_type != 8 &&
        msg_type != 9 && msg_type != 11) {
        return;
    }

    int forward_message_len =
            sizeof(t_relay_forward_message) + sizeof(t_relay_option_relay_message) + ntohs(udp_h->uh_ulen) -
            UDP_HDRLEN + sizeof(t_relay_option_ll_address) + sizeof(t_relay_option_relay_message) +
            strlen(interface_name);
    t_relay_forward_message *forward_message = (t_relay_forward_message *) malloc(forward_message_len);

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

    t_relay_option_relay_message *o_interface_id = (t_relay_option_relay_message *) ((char *) o_link_layer +
                                                                                     sizeof(t_relay_option_ll_address));
    o_interface_id->option_code = htons(OPTION_INTERFACE_ID);
    o_interface_id->length = htons(strlen(interface_name));
    memcpy(o_interface_id->option_data, interface_name, strlen(interface_name));

    char ip6_peer[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &ip6_h->saddr, ip6_peer, INET6_ADDRSTRLEN);
    string peer_addr = ip6_peer;

    string mac_addr = (ether_ntoa((const struct ether_addr *) &eth_h->ether_shost));
    mac_addr_map.insert({peer_addr, mac_addr});

    int sock_send_to_server;

    if ((sock_send_to_server = socket(PF_INET6, SOCK_DGRAM, 0)) < 0) {
        err(1, "can't create socket");
    }


    struct sockaddr_in6 server;
    memset(&server, 0, sizeof(server));
    server.sin6_family = AF_INET6;
    inet_pton(AF_INET6, args_cli.server_arg, &server.sin6_addr);
    socklen_t server_length = sizeof(server);

    server.sin6_port = htons(PORT);

    // send relay_forward
    if (sendto(sock_send_to_server, forward_message, forward_message_len, 0, (struct sockaddr *) &server,
               server_length) < 0) {
        err(1, "sento()");
    }
}

void process_msg_from_server(const char *buffer, int read_count) {
    t_relay_forward_message *mess_relay_reply = (t_relay_forward_message *) (buffer);
    if (mess_relay_reply->msg_type != 13) {
        err(1, "wrong response from server, expected msg_type 13, got %d", mess_relay_reply->msg_type);
    }

    int i = sizeof(t_relay_forward_message);
    char interface_name[16] = {};

    t_relay_option_relay_message *option = (t_relay_option_relay_message *) (buffer +
                                                                             sizeof(t_relay_forward_message));
    t_relay_option_relay_message *relay_message = NULL;
    while (true) {
        switch (ntohs(option->option_code)) {
            case OPTION_INTERFACE_ID:
                memcpy(interface_name, option->option_data, ntohs(option->length));
                break;
            case OPTION_RELAY_MSG:
                relay_message = option;
                break;
        }
        i += ntohs(option->length);
        if (i > read_count || (relay_message && interface_name[0] != '\0'))
            break;
        option = (t_relay_option_relay_message *) ((char *) option + sizeof(t_relay_option_relay_message) +
                                                   ntohs(option->length));
    }

    if (!relay_message)
        err(1, "Don't have enough information from server");

    uint8_t msg_type;
    memcpy(&msg_type, relay_message->option_data, 1);
    if (msg_type == 7) {
        char ip6[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &mess_relay_reply->peer_addr, ip6, INET6_ADDRSTRLEN);
        string peer_addr = ip6;

        inet_ntop(AF_INET6, ((char *) relay_message->option_data + 24), ip6, INET6_ADDRSTRLEN);
        string output = mac_addr_map.find(peer_addr)->second + ',' + ip6;
        if (args_cli.debug_enabled)
            cout << output << endl;
        if (args_cli.log_enabled)
            syslog(LOG_INFO, "%s", output.c_str());

    }
    int socket_send_to_client;
    if ((socket_send_to_client = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
        err(1, "socket()");

    struct sockaddr_in6 address{.sin6_family = AF_INET6, .sin6_port = htons(PORT_CLIENT)};

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));

    if (interface_name[0] != '\0') {
        memcpy(ifr.ifr_name, interface_name, sizeof(ifr.ifr_name));
        if (setsockopt(socket_send_to_client, SOL_SOCKET, SO_BINDTODEVICE, (void *) &ifr, sizeof(ifr)) < 0) {
            err(1, "setsockopt()");
        }
    }

    memcpy(&address.sin6_addr, &mess_relay_reply->peer_addr, 16);

    if (sendto(socket_send_to_client, relay_message->option_data,
               ntohs(relay_message->length), 0, (struct sockaddr *) &address, sizeof(address)) < 0)
        err(1, "sendto() client");
    close(socket_send_to_client);
}

void server_service(struct sockaddr_in6 server) {
    socklen_t server_length = sizeof(server);
    int sock_recv_from_server;

    if ((sock_recv_from_server = socket(PF_INET6, SOCK_DGRAM, 0)) < 0) {
        err(1, "can't create socket");
    }

    struct sockaddr_in6 srcaddr = {.sin6_family = AF_INET6, .sin6_port = htons(PORT)};
    srcaddr.sin6_addr = in6addr_any;


    if (bind(sock_recv_from_server, (struct sockaddr *) &srcaddr, sizeof(srcaddr)) < 0) {
        err(1, "bind()");
    }

    char buffer[BUFF_SIZE] = {0,};
    int read_count;

    // recv relay_reply from server
    while (true) {
        if ((read_count =
                     recvfrom(sock_recv_from_server, (char *) buffer, sizeof(buffer), 0, (struct sockaddr *) &server,
                              &server_length)) < 0) {
            err(1, "rcvfrom()");
        }
        process_msg_from_server(buffer, read_count);
    }
    close(sock_recv_from_server);
}

struct udphdr *get_udph_from_ipv6h(struct ipv6hdr *ip6hdr) {

    int ip_next_header_len = 0, extension_length = IP6_HDRLEN;
    while (ip6hdr->nexthdr != IPPROTO_UDP) {
        ip6hdr = (struct ipv6hdr *) ((char *) ip6hdr + extension_length);
        memcpy(&extension_length, (char *) ip6hdr + 1, 1);
        ip_next_header_len += extension_length;

        if (ip_next_header_len > 1000) {
            err(1, "Ipv6 header extension error");
        }
    }
    return ((struct udphdr *) ((char *) ip6hdr + extension_length));
}
