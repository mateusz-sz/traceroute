#include <netinet/ip_icmp.h>
#include <assert.h>
#include <unistd.h>
#include <strings.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "icmp_checksum.h"
#include "icmp_send.h"
#include "error_handler.h"

struct icmp create_icmp_echo_request (uint16_t seq_number)
{
    struct icmp header;
    header.icmp_type = ICMP_ECHO;
    header.icmp_code = 0;
    header.icmp_hun.ih_idseq.icd_id = (uint16_t) getpid();
    header.icmp_hun.ih_idseq.icd_seq = seq_number;
    header.icmp_cksum = 0;
    header.icmp_cksum = compute_icmp_checksum((uint16_t*)&header, sizeof(header));

    return header;
}

struct sockaddr_in ip2sockaddr_in (char* recipient_ip)
{
    struct sockaddr_in recipient;
    bzero (&recipient, sizeof(recipient));
    recipient.sin_family = AF_INET;
    inet_pton(AF_INET, recipient_ip, &recipient.sin_addr);

    return recipient;
}

void send_icmp_echo_request (int sockfd, int ttl, char* recipient_ip, uint16_t sequential_number)
{
    struct sockaddr_in recipient = ip2sockaddr_in(recipient_ip);

    int ttl_set = setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));
    if (ttl_set < 0) {
        handle_error("setsockopt error");
    }

    struct icmp packet = create_icmp_echo_request(sequential_number);
    ssize_t bytes_sent = sendto(
        sockfd,
        &packet,
        sizeof(packet),
        0,
        (struct sockaddr*)&recipient,
        sizeof(recipient)
    );
    if (bytes_sent < 0) {
        handle_error("sendto error");
    }
}
