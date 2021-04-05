#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#include "icmp_retrieve.h"
#include "error_handler.h"

struct icmp* decapsulate_icmp_from_ip_packet(struct ip* ip_header)
{
    ssize_t ip_header_len = 4 * ip_header->ip_hl;
    uint8_t* ip_bits = (uint8_t*) ip_header;
    u_int8_t *icmp_packet =  ip_bits + ip_header_len;

    struct icmp *icmp_header = (struct icmp *) icmp_packet;

    return icmp_header;
}

struct IpHeaderAndSenderIP* get_ip_with_sender_from_socket(int sockfd)
{
    struct sockaddr_in 	            sender;	
    socklen_t 			            sender_len = sizeof(sender);
    u_int8_t 			            buffer[IP_MAXPACKET];

    ssize_t packet_len = recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr*)&sender, &sender_len);
    if (packet_len < 0) {
        handle_error("recvfrom error");
    }

    char ip_str[20]; // sender IP
    inet_ntop(AF_INET, &(sender.sin_addr), ip_str, sizeof(ip_str));

    struct ip* ip_header = (struct ip*) buffer;

    struct IpHeaderAndSenderIP* data = (struct IpHeaderAndSenderIP*) malloc(sizeof(struct IpHeaderAndSenderIP));
    if (data == NULL) {
        handle_error("malloc error");
    }

    strcpy(data->sender_ip, ip_str);
    data->ip_header = ip_header;

    return data;   
}

struct IcmpHeaderAndSenderIP* get_icmp_with_sender_from_socket(int sockfd)
{
    struct IpHeaderAndSenderIP* ip_with_sender = get_ip_with_sender_from_socket(sockfd);
    struct IcmpHeaderAndSenderIP* data = (struct IcmpHeaderAndSenderIP*) malloc(sizeof(struct IcmpHeaderAndSenderIP));
    if (data == NULL) {
        handle_error("malloc error");
    }

    strcpy(data->sender_ip, ip_with_sender->sender_ip);
    data->icmp_header = decapsulate_icmp_from_ip_packet(ip_with_sender->ip_header);

    free(ip_with_sender);

    return data;
}