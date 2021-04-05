#ifndef ICMP_SEND_MODULE
#define ICMP_SEND_MODULE

#include <stdint.h>

struct icmp create_icmp_echo_request (uint16_t seq_number);

struct sockaddr_in ip2sockaddr_in (char* recipient_ip);

void send_icmp_echo_request (
    int sockfd,
    int ttl,
    char* recipient_ip,
    uint16_t sequential_number
);

#endif