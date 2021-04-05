#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>

#include <sys/types.h>
#include <netinet/ip_icmp.h>
#include <strings.h>
#include <stdbool.h>
#include <unistd.h>

#include<netinet/ip.h>
#include<sys/socket.h>
#include<sys/ioctl.h>
#include<sys/time.h>
#include <time.h>

#include "icmp_send.h"
#include "icmp_retrieve.h"


bool is_valid_ip_address(char *ip_address)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip_address, &(sa.sin_addr));
    return result == 1;
}


int main(int argc, char** argv)
{
    char* target_ip;

    if (argc == 2 && is_valid_ip_address(argv[1])) {
        target_ip = argv[1];
    }
    else if (argc == 0) {
        printf("Specify \"host\" missing argument.\n");
        return EXIT_FAILURE;
    }
    else {
        fprintf(stderr, "The only argument of a program must be a valid IPv4 address. Terminating.\n");
        return EXIT_FAILURE;
    }

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        printf("Could not open socket.\n");
        fprintf(stderr, "socket error: %s\n", strerror(errno)); 
        return EXIT_FAILURE;
    }

    bool got_echo_reply = false;
    for (int ttl=1; ttl<=30; ttl++) {

        struct timeval send_times[3];
        for (int j=0; j<3; j++) {
            int gettime = gettimeofday(&send_times[j], NULL);
            if (gettime != 0) {
                fprintf(stderr, "gettimeofday error: %s\n", strerror(errno));
                return EXIT_FAILURE;
            }

            send_icmp_echo_request(sockfd, ttl, target_ip, j+1);
        }

        char response_sender_ip[20];
        bool if_received_packets[3] = {false, false, false};
        int rtts[3];
        struct timeval timeout; timeout.tv_sec = 1; timeout.tv_usec = 0;
        for (int packet_nr=0; packet_nr<3; packet_nr++) {

            fd_set descriptors;
            FD_ZERO (&descriptors);
            FD_SET (sockfd, &descriptors);
            
            int ready = select (sockfd+1, &descriptors, NULL, NULL, &timeout);
            if (ready < 0) {
                fprintf(stderr, "select error: %s\n", strerror(errno));
                return EXIT_FAILURE;
            }
            else if (ready == 0) {
                if_received_packets[packet_nr] = false;
                continue;
            }
            else {

                

                struct IcmpHeaderAndSenderIP* packet_with_sender = get_icmp_with_sender_from_socket(sockfd);
                if (packet_with_sender == NULL) {
                    fprintf(stderr, "Could not retrieve packet with sender. Error: %s", strerror(errno));
                    return EXIT_FAILURE;
                }

                struct timeval now, rtt;
                if (gettimeofday(&now, NULL) != 0) {
                    free(packet_with_sender);
                    fprintf(stderr, "gettimeofday error: %s\n", strerror(errno));
                    return EXIT_FAILURE;
                }

                strcpy(
                    response_sender_ip,
                    packet_with_sender->sender_ip
                );

                struct icmp* icmp_header = packet_with_sender->icmp_header;

                if (icmp_header->icmp_type == ICMP_TIME_EXCEEDED) {

                    struct ip* ip_packet_sent = (struct ip*) ((uint8_t*) icmp_header + 8);
                    struct icmp* sent_icmp_header = decapsulate_icmp_from_ip_packet(ip_packet_sent);

                    if (sent_icmp_header->icmp_type == ICMP_ECHO) {

                        if (sent_icmp_header->icmp_id == getpid()) {
                            uint16_t sequential_id = sent_icmp_header->icmp_hun.ih_idseq.icd_seq;
                            if_received_packets[sequential_id-1] = true;

                            timersub(&now, &send_times[sequential_id-1], &rtt);
                            rtts[sequential_id-1] = (int) (rtt.tv_sec * 1000 + rtt.tv_usec / 1000);
                        }
                        else {
                            free(packet_with_sender);
                            packet_nr--; continue;
                        }
                    }

                }
                else if (icmp_header->icmp_type == ICMP_ECHOREPLY) {

                    if (icmp_header->icmp_id == getpid()) {
                        got_echo_reply = true;
                        uint16_t sequential_id = icmp_header->icmp_hun.ih_idseq.icd_seq;

                        if_received_packets[sequential_id-1] = true;

                        timersub(&now, &send_times[sequential_id-1], &rtt);
                        rtts[sequential_id-1] = (int) (rtt.tv_sec * 1000 + rtt.tv_usec / 1000);
                    }
                    else {
                        free(packet_with_sender);
                        packet_nr--; continue;
                    }
                }
                else {
                    free(packet_with_sender);
                    packet_nr--; continue;
                }

                free(packet_with_sender);
            }
        }

        if (
            if_received_packets[0] ||
            if_received_packets[1] ||
            if_received_packets[2]
        ) {
            printf("%d. %s ", ttl, response_sender_ip);
            if (
                if_received_packets[0] &&
                if_received_packets[1] &&
                if_received_packets[2]
            ) {
                printf("%dms\n", (rtts[0] + rtts[1] + rtts[2]) / 3);
            }
            else {
                printf("???\n");
            }
        }
        else {
            printf("%d. *\n", ttl);
        }

        if (got_echo_reply) {
            return EXIT_SUCCESS;
        }
    }

    return EXIT_SUCCESS;
}
