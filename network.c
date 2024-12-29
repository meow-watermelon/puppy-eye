#include <limits.h>
#include <stdio.h>
#include "network.h"

int get_interface_metrics(struct network_metrics *input_network_metrics) {
    /* interface count */
    int count = 0;

    /* plaxceholder for unused fields */
    long int unused;

    FILE *net_dev_file;
    net_dev_file = NULL;

    /* read line buffer variable */
    char line[BUFSIZ];

    /* read /proc/net/dev file */
    net_dev_file = fopen("/proc/net/dev", "r");
    if (net_dev_file == NULL) {
        goto handle_error;
    }

    /* skip first 2 lines */
    fgets(line, sizeof(line), net_dev_file);
    fgets(line, sizeof(line), net_dev_file);

    while (fgets(line, sizeof(line), net_dev_file) != NULL) {
        int ret_sscanf = sscanf(line, " %[^:]: %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
            input_network_metrics->if_network[count].interface_name,
            &input_network_metrics->if_network[count].rx_bytes,
            &input_network_metrics->if_network[count].rx_packets,
            &input_network_metrics->if_network[count].rx_errors,
            &input_network_metrics->if_network[count].rx_dropped,
            &unused, &unused, &unused, &unused,
            &input_network_metrics->if_network[count].tx_bytes,
            &input_network_metrics->if_network[count].tx_packets,
            &input_network_metrics->if_network[count].tx_errors,
            &input_network_metrics->if_network[count].tx_dropped,
            &unused,
            &input_network_metrics->if_network[count].collisions,
            &unused, &unused
        );

        if (ret_sscanf < 17 || ret_sscanf == EOF) {
            fprintf(stderr, "ERROR: unable to parse line %s\n", line);
            continue;
        } else {
            ++count;
        }

        /* stop parsing if number of interfaces is greater than INTERFACE_COUNT */
        if (count >= INTERFACE_COUNT) {
            fprintf(stderr, "ERROR: number of network interfaces is greater than %d\n", INTERFACE_COUNT);
            break;
        }
    }

    /* close file handle */
    fclose(net_dev_file);

    return count;

handle_error:
    if (net_dev_file != NULL) {
        fclose(net_dev_file);
    }

    return -1;
}

int get_arp_metrics(struct network_metrics *input_network_metrics) {
    /* ARP entries count */
    input_network_metrics->arp_cache_entries = 0;

    FILE *net_arp_file;
    net_arp_file = NULL;

    /* read line buffer variable */
    char line[BUFSIZ];

    /* read /proc/net/arp file */
    net_arp_file = fopen("/proc/net/arp", "r");
    if (net_arp_file == NULL) {
        goto handle_error;
    }

    /* skip first line */
    fgets(line, sizeof(line), net_arp_file);

    while (fgets(line, sizeof(line), net_arp_file) != NULL) {
        ++input_network_metrics->arp_cache_entries;
    }

    /* close file handle */
    fclose(net_arp_file);

    return 0;

handle_error:
    if (net_arp_file != NULL) {
        fclose(net_arp_file);
    }

    return -1;
}
