#ifndef ICMP_RETRIEVE_MODULE
#define ICMP_RETRIEVE_MODULE

#include <netinet/ip_icmp.h>

struct IpHeaderAndSenderIP {
    char sender_ip[20];
    struct ip* ip_header;
};

struct IcmpHeaderAndSenderIP {
    char sender_ip[20];
    struct icmp* icmp_header;
};

struct icmp* decapsulate_icmp_from_ip_packet (struct ip* ip_header);

struct IpHeaderAndSenderIP* get_ip_with_sender_from_socket(int sockfd);

struct IcmpHeaderAndSenderIP* get_icmp_with_sender_from_socket(int sockfd);

#endif