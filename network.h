#ifndef NETWORK_H
#define NETWORK_H

#include <net/if.h>

#define INTERFACE_COUNT 64 

struct interfaces {
    char interface_name[IFNAMSIZ];
    long int rx_bytes;
    long int rx_packets;
    long int rx_errors;
    long int rx_dropped;
    long int tx_bytes;
    long int tx_packets;
    long int tx_errors;
    long int tx_dropped;
    long int collisions;
};

struct network_metrics {
    /* interfaces metrics */
    struct interfaces if_network[INTERFACE_COUNT];

    /* ARP metrics */
    int arp_cache_entries;
};

extern int get_interface_metrics(struct network_metrics *input_network_metrics);
extern int get_arp_metrics(struct network_metrics *input_network_metrics);

#endif /* NETWORK_H */
