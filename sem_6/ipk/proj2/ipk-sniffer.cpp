#include <cstdarg>
#include <netdb.h>
#include "ipk-sniffer.h"

#define ETHERNET_HDRLEN (14)    // Ethernet header length
#define IP6_HDRLEN (40)         // IPv6 header length
#define UDP_HDRLEN  (8)         // UDP header length, excludes data

#define ERR_ARG 1
#define ERR_ALLOC 2
#define ERR_PCAP_FUNC 3

#define PROMISCUOUS_ON (1)


using namespace std;


void print_error_exit(int status, const char *fmt, ...);

void print_usage();

t_arguments process_cli_arguments(int argc, char *const *argv);

void print_all_interfaces(pcap_if_t *all_interfaces);

void sniff_for_client(pcap_t *handle, const char *capture_filter);

string build_filter();

string add_udp_if_needed(string filter);

string add_port_if_needed(string filter);

void print_packet_content(unsigned char *packet, unsigned int length);

int packet_counter = 0;
t_arguments args_cli = arguments_default;

int main(int argc, char *argv[]) {
    pcap_if_t *all_interfaces;
    char errbuff[PCAP_ERRBUF_SIZE];

    args_cli = process_cli_arguments(argc, argv);
    if (args_cli.num == -1)
        args_cli.num = 1;

    if (pcap_findalldevs(&all_interfaces, errbuff) == -1) {
        print_error_exit(ERR_PCAP_FUNC, "Error in pcap_findalldevs: %s\n", errbuff);
    }

    if (args_cli.interface == NULL) {
        print_all_interfaces(all_interfaces);
        return 0;
    }

    pcap_t *interface;
    unsigned int mask;
    unsigned int address;

    pcap_lookupnet(args_cli.interface, &address, &mask, errbuff);

    // open device for sniffing
    interface = pcap_open_live(args_cli.interface, BUFSIZ, PROMISCUOUS_ON, -1, errbuff);
    if (interface == NULL) {
        print_error_exit(ERR_PCAP_FUNC, "pcap_open_live() failed due to [%s]\n", errbuff);
    }

    string filter;
    filter = build_filter();

    sniff_for_client(interface,filter.c_str());


    return 0;
}

string add_tcp_if_needed(string filter) {
    if (args_cli.isTcp) {
        if (filter.empty()) {
            filter = "tcp";
        } else {
            filter += " or tcp";
        }

        filter = add_port_if_needed(filter);
    }



    return filter;
}

string build_filter() {
    string filter;
    filter = add_udp_if_needed("");
    filter = add_tcp_if_needed(filter);
    if (!args_cli.isTcp && !args_cli.isUdp) {
        filter = add_port_if_needed(filter);
    }
    return filter;
}

string add_port_if_needed(string filter) {
    if (args_cli.port != -1) {
        if (filter.empty()) {
            filter = "port " + to_string(args_cli.port);
        } else {
            filter += " port " + to_string(args_cli.port);
        }
    }
    return filter;
}

string add_udp_if_needed(string filter) {
    if (args_cli.isUdp) {
        if (filter.empty()) {
            filter = "udp";
        } else {
            filter += " or udp";
        }
        filter = add_port_if_needed(filter);
    }

    return filter;
}


void print_all_interfaces(pcap_if_t *all_interfaces) {
    pcap_if_t *d;
    printf("You need to specify interface! Here is list of available devices on your system:\n\n");
    for (d = all_interfaces; d; d = d->next) {
        printf("%-15s ---> Description: ", d->name);
        if (d->description)
            printf(" (%s)\n", d->description);
        else
            printf(" (No description available)\n");
    }
}

void packet_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
    struct ip *ip_header;
    struct ether_header *ether_header;
    const struct tcphdr *tcp_header;
    const struct udphdr *udp_header;
    u_int size_ip;

    packet_counter++;

    //do not show packets
    if (args_cli.num != -1 && args_cli.num < packet_counter) {
        return;
    }

    char *time = ctime((const time_t *) &header->ts.tv_sec);
    if (time[strlen(time) - 1] == '\n') // delete newline from time
        time[strlen(time) - 1] = '\0';

    unsigned int packet_length = header->len;

    ether_header = (struct ether_header *) packet;

    unsigned char packet_array[packet_length];


    switch (ntohs(ether_header->ether_type)) {
        case ETHERTYPE_IP: // IPv4 packet
            ip_header = (struct ip *) (packet + ETHERNET_HDRLEN);
            size_ip = ip_header->ip_hl * 4;

            switch (ip_header->ip_p) {
                case 6: // TCP
                    tcp_header = (struct tcphdr *) (packet + ETHERNET_HDRLEN + size_ip);

                    printf("%s %s : %d > %s : %d", time, inet_ntoa(ip_header->ip_src), ntohs(tcp_header->th_sport), inet_ntoa(ip_header->ip_dst), ntohs(tcp_header->th_dport));
                    mempcpy(packet_array, packet, packet_length);
                    print_packet_content(packet_array, packet_length);
                    printf("\n\n");
                    break;
                case 17: // UDP
                    udp_header = (struct udphdr *) (packet + ETHERNET_HDRLEN + size_ip);

                    printf("%s %s : %d > %s : %d", time, inet_ntoa(ip_header->ip_src), ntohs(udp_header->uh_sport), inet_ntoa(ip_header->ip_dst), ntohs(udp_header->uh_dport));
                    mempcpy(packet_array, packet, packet_length);
                    print_packet_content(packet_array, packet_length);
                    printf("\n\n");
                    break;
            }

            break;

        case ETHERTYPE_IPV6:  // IPv6
            struct ipv6hdr *ip6_h;
            ip6_h = (struct ipv6hdr *) (packet + ETHERNET_HDRLEN);
            char ip6_src_s[INET6_ADDRSTRLEN], ip6_dst_s[INET6_ADDRSTRLEN];
            memset(ip6_src_s, 0, sizeof(ip6_src_s));
            memset(ip6_dst_s, 0, sizeof(ip6_dst_s));
            inet_ntop(AF_INET6, &ip6_h->saddr, ip6_src_s, sizeof(ip6_src_s));
            inet_ntop(AF_INET6, &ip6_h->daddr, ip6_dst_s, sizeof(ip6_dst_s));

            switch (ip6_h->nexthdr) {
                case 6: // TCP
                    tcp_header = (struct tcphdr *) (ip6_h + IP6_HDRLEN);

                    printf("%s %s : %d > %s : %d", time, ip6_src_s, ntohs(tcp_header->th_sport), ip6_dst_s, ntohs(tcp_header->th_dport));
                    mempcpy(packet_array, packet, packet_length);
                    print_packet_content(packet_array, packet_length);
                    printf("\n\n");
                    break;
                case 17: // UDP
                    udp_header = (struct udphdr *) (ip6_h + IP6_HDRLEN);

                    printf("%s %s : %d > %s : %d", time, ip6_src_s, ntohs(udp_header->uh_sport), ip6_dst_s, ntohs(udp_header->uh_dport));
                    mempcpy(packet_array, packet, packet_length);
                    print_packet_content(packet_array, packet_length);
                    printf("\n\n");
                    break;
            }
    }
}

void print_packet_content(unsigned char *packet, unsigned int length) {
    cout << endl;
    string contentString = "";
    for (int i = 0; i < length; i++) {
        if (i % 16 == 0) {

            cout << "  " << contentString << endl;
            contentString = "";
            printf("0x%.8X", i);
        }

        printf(" %.2X", packet[i]);
        if (isprint(packet[i]))
            contentString += packet[i];
        else
            contentString += '.';
    }
    cout << endl;
}

void sniff_for_client(pcap_t *handle, const char *capture_filter) {
    struct bpf_program fp;

    if (pcap_compile(handle, &fp, capture_filter, 0, 0) == -1)
        print_error_exit(ERR_PCAP_FUNC, "pcap_compile() failed");

    if (pcap_setfilter(handle, &fp) == -1)
        print_error_exit(ERR_PCAP_FUNC, "pcap_setfilter() failed");

    if (pcap_loop(handle, -1, packet_handler, (u_char *) handle) == -1)
        print_error_exit(ERR_PCAP_FUNC, "pcap_loop() failed");

    pcap_close(handle);

}

t_arguments process_cli_arguments(int argc, char *const *argv) {

    static struct option long_options[] =
            {
                    {"port", optional_argument, NULL, 'p'},
                    {"tcp",  optional_argument, NULL, 't'},
                    {"udp",  optional_argument, NULL, 'u'},
                    {NULL, 0,                   NULL, 0}
            };
    int ch;
    char *eptr;
    while ((ch = getopt_long(argc, argv, "i:p:n:tu", long_options, NULL)) != -1) {
        switch (ch) {
            case 'i':
                args_cli.interface = optarg;
                break;
            case 'p':
                if (optarg == NULL) {
                    print_error_exit(ERR_ARG, "Wrong format of program arguments");
                }
                args_cli.port = (int) strtol(optarg, &eptr, 10);

                //test for error
                if (*eptr != '\0') {
                    print_error_exit(ERR_ARG, "Can't parse argument of parameter [-p|--port]");

                }
                if (args_cli.port < 0 || args_cli.port > 65353)
                    print_error_exit(ERR_ARG, "Argument of parameter [-p|--port] has to be in range <0,65353>");
                break;

            case 'n':
                args_cli.num = (int) strtol(optarg, &eptr, 10);

                //test for error
                if (*eptr != '\0') {
                    print_error_exit(ERR_ARG, "Can't parse argument of parameter [-n|--num]");

                }
                if (args_cli.num < 1 || errno == ERANGE)
                    print_error_exit(ERR_ARG, "Argument of parameter [-n|--num] has to be in range <1,2147483647>");
                break;
            case 't':
                args_cli.isTcp = true;
                break;
            case 'u':
                args_cli.isUdp = true;
                break;
            case '?':
                print_error_exit(ERR_ARG, "Unknown argument");
                break;
            default:
                print_error_exit(ERR_ARG, "Unknown argument");
        }
    }

    // option index
    if (optind < argc) {
        print_error_exit(ERR_ARG, "ERROR unrecognized extra argument");
    }

    return args_cli;
}

void print_usage() {
    fprintf(stdout, "USAGE:# ./ipk-sniffer -i interface [-p port] [--tcp|-t] [--udp|-u] [-n num]\n");
}

void print_error_exit(int status, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "ERROR : ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);


    if (status == ERR_ARG) {
        print_usage();
    }

    exit(status);
}

