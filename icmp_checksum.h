#ifndef ICMP_CHECKSUM_MODULE
#define ICMP_CHECKSUM_MODULE

#include <stdlib.h>

u_int16_t compute_icmp_checksum (const void *buff, int length);

#endif