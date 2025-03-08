#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>
#include "os.h"
#include "memory.h"
#include "network.h"
#include "disk.h"
#include "ncurses_utils.h"
#include "utils.h"

#define SIZEOF(x) (sizeof(x) / sizeof(x[0]))
#define VERSION "1.2.0"

/* define usage function */
static void usage(void) {
    printf(
        "Puppy Eye Linux System Monitoring Utility - Version %s\n"
        "usage: puppy-eye [-r|--refresh <second(s)>]\n"
        "                 [-h|--help]\n", VERSION
    );
}

int main(int argc, char *argv[]) {
    /* define command-line options */
    char *short_opts = "r:h";
    struct option long_opts[] = {
        {"refresh", required_argument, NULL, 'r'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    long int refresh_second = 5;
    int error_flag = 0;
    char error_msg[BUFSIZ];
    int exit_code = EXIT_SUCCESS;

    /* suppress default getopt error messages */
    opterr = 0;

    int c;

    while (1) {
        c = getopt_long(argc, argv, short_opts, long_opts, NULL);

        if (c == -1) {
            break;
        }

        switch (c) {
            case 'r':
                errno = 0;
                refresh_second = strtol(optarg, NULL, 10);

                if (errno != 0) {
                    fprintf(stderr, "ERROR: failed to convert refresh second value\n\n");
                    exit(EXIT_FAILURE);
                }

                if (refresh_second <= 0) {
                    fprintf(stderr, "ERROR: refresh second must be an integer and greater than 0\n\n");
                    usage();
                    exit(EXIT_FAILURE);
                }

                break;
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case '?':
                fprintf(stderr, "ERROR: Unknown option\n\n");
                usage();
                exit(EXIT_FAILURE);
            default:
                fprintf(stderr, "ERROR: Unimplemented option\n\n");
                usage();
                exit(EXIT_FAILURE);
        }
    }

    /* check if host OS is Linux */
    if (is_linux() != 1) {
        fprintf(stderr, "ERROR: Puppy Eye can be only running on Linux operating system\n");
        exit(EXIT_FAILURE);
    }

    /* initialize metric structs */
    struct os_metrics cur_os_metrics;
    struct os_metrics prev_os_metrics;

    struct memory_metrics cur_memory_metrics;
    struct memory_metrics prev_memory_metrics;

    struct network_metrics cur_network_metrics;
    struct network_metrics prev_network_metrics;

    struct disk_metrics cur_disk_metrics;
    struct disk_metrics prev_disk_metrics;

    /* previous data copy state flag */
    int prev_data_flag = 0;

    /* initialize previous network interface return value */
    int prev_ret_get_interface_metrics;

    /* initialize beginning row number for network interface */
    int init_if_name_row = 21;

    /* initialize previous disk metrics return value */
    int prev_ret_get_disk_metrics;

    /* delimiter positions */
    int network_column_positions[] = {19, 33, 45, 58, 73, 87, 99, 112, 127};
    int disk_column_positions[] = {14, 29, 46, 62};

    /* initialize ncurses window struct */
    WINDOW *main_window;
    main_window = NULL;

    /* initialize screen */
    initscr();

    /* disable line buffering */
    cbreak();

    /* set cursor state to invisiable */
    curs_set(0);

    /* create window */
    main_window = newwin(0, 0, 0, 0);

    if (main_window == NULL) {
        fprintf(stderr, "ERROR: failed to create ncurses window\n");
        endwin();
        exit(EXIT_FAILURE);
    }

    /* enable non-blocking input */
    nodelay(main_window, TRUE);

    /* data collection and refresh logic */
    while (1) {
        if (wgetch(main_window) == 'q') {
            break;
        }

        /* clear window */
        wclear(main_window);
        wrefresh(main_window);

        /* create window boarder */
        box(main_window, 0, 0);
        wrefresh(main_window);

        /* initialize variables */
        int ret_get_loadavg;
        int ret_get_fd_usage;
        int ret_get_process_states;

        int ret_get_memory_usage;

        int ret_get_interface_metrics;
        int ret_get_arp_metrics;

        int ret_get_disk_metrics;

        int if_name_found = 0;
        int disk_name_found = 0;

        /* retrieve metrics for os_metrics */
        ret_get_loadavg = get_loadavg(&cur_os_metrics);
        if (ret_get_loadavg < 0) {
            strcpy(error_msg, "ERROR: unable to retrieve loadavg metrics");
            ++error_flag;
            break;
        }

        ret_get_fd_usage = get_fd_usage(&cur_os_metrics);
        if (ret_get_fd_usage < 0) {
            strcpy(error_msg, "ERROR: unable to retrieve file descriptor metrics");
            ++error_flag;
            break;
        }

        ret_get_process_states = get_process_states(&cur_os_metrics);
        if (ret_get_process_states < 0) {
            strcpy(error_msg, "ERROR: unable to retrieve process states metrics");
            ++error_flag;
            break;
        }

        get_current_users(&cur_os_metrics);

        /* retrieve metrics for memory_metrics */
        ret_get_memory_usage = get_memory_usage(&cur_memory_metrics);
        if (ret_get_memory_usage < 0) {
            strcpy(error_msg, "ERROR: unable to retrieve memory metrics");
            ++error_flag;
            break;
        }

        /* retrieve metrics for network_metrics */
        ret_get_interface_metrics = get_interface_metrics(&cur_network_metrics);
        if (ret_get_interface_metrics < 0) {
            strcpy(error_msg, "ERROR: unable to retrieve network interface metrics");
            ++error_flag;
            break;
        }

        ret_get_arp_metrics = get_arp_metrics(&cur_network_metrics);
        if (ret_get_arp_metrics < 0) {
            strcpy(error_msg, "ERROR: unable to retrieve arp metrics");
            ++error_flag;
            break;
        }

        /* retrieve metrics for disk_metrics */
        ret_get_disk_metrics = get_disk_metrics(&cur_disk_metrics);
        if (ret_get_disk_metrics < 0) {
            strcpy(error_msg, "ERROR: unable to retrieve disk metrics");
            ++error_flag;
            break;
        }

        /* construct window layout */
        construct_window_layout(main_window, ret_get_interface_metrics);

        /* print available metrics */
        mvwprintw(main_window, 3, 36, "%6.2f", cur_os_metrics.loadavg_1m);
        mvwprintw(main_window, 3, 43, "%6.2f", cur_os_metrics.loadavg_5m);
        mvwprintw(main_window, 3, 50, "%6.2f", cur_os_metrics.loadavg_15m);
        mvwprintw(main_window, 3, 83, "%10ld", cur_os_metrics.fd_usage);
        mvwprintw(main_window, 4, 12, "%8ld", cur_os_metrics.total_process);
        mvwprintw(main_window, 4, 29, "%8ld", cur_os_metrics.running_process);
        mvwprintw(main_window, 4, 48, "%8ld", cur_os_metrics.blocked_process);
        mvwprintw(main_window, 4, 67, "%8ld", cur_os_metrics.zombie_process);
        mvwprintw(main_window, 5, 8, "%8ld", cur_os_metrics.current_users);

        mvwprintw(main_window, 10, 19, "%10ld", cur_memory_metrics.total_memory / 1024);
        mvwprintw(main_window, 10, 38, "%10ld", cur_memory_metrics.avail_memory / 1024);
        mvwprintw(main_window, 10, 57, "%10ld", cur_memory_metrics.free_memory / 1024);
        mvwprintw(main_window, 10, 75, "%10ld", cur_memory_metrics.buffer / 1024);
        mvwprintw(main_window, 10, 95, "%10ld", cur_memory_metrics.cache / 1024);
        mvwprintw(main_window, 10, 114, "%10ld", cur_memory_metrics.page_tables / 1024);
        mvwprintw(main_window, 11, 17, "%10ld", cur_memory_metrics.total_swap / 1024);
        mvwprintw(main_window, 11, 36, "%10ld", cur_memory_metrics.free_swap / 1024);

        mvwprintw(main_window, 17, 20, "%8d", cur_network_metrics.arp_cache_entries);

        /* print other metrics that need time difference */
        if (prev_data_flag > 0) {
            mvwprintw(main_window, 3, 113, "%10ld", (cur_os_metrics.context_switches - prev_os_metrics.context_switches) / refresh_second);

            mvwprintw(main_window, 12, 20, "%12ld", (cur_memory_metrics.major_page_faults - prev_memory_metrics.major_page_faults) / refresh_second);
            mvwprintw(main_window, 12, 57, "%12ld", (cur_memory_metrics.minor_page_faults - prev_memory_metrics.minor_page_faults) / refresh_second);

            /* network interface metrics*/
            for (int i = 0; i < ret_get_interface_metrics; ++i) {
                for (int j = 0; j < prev_ret_get_interface_metrics; ++j) {
                    /* only process if current interface name exists */
                    if (strcmp(cur_network_metrics.if_network[i].interface_name, prev_network_metrics.if_network[i].interface_name) == 0) {
                        ++if_name_found;
                        print_delimiter(main_window, init_if_name_row + if_name_found - 1, network_column_positions, SIZEOF(network_column_positions));

                        /* print interface metrics */
                        mvwprintw(main_window, init_if_name_row + if_name_found - 1, 1, "%-16s", cur_network_metrics.if_network[i].interface_name);
                        mvwprintw(main_window, init_if_name_row + if_name_found - 1, 23, "%10ld", (cur_network_metrics.if_network[i].rx_packets - prev_network_metrics.if_network[i].rx_packets) / refresh_second);
                        mvwprintw(main_window, init_if_name_row + if_name_found - 1, 35, "%10ld", (cur_network_metrics.if_network[i].rx_bytes - prev_network_metrics.if_network[i].rx_bytes) / 1024 / refresh_second);
                        mvwprintw(main_window, init_if_name_row + if_name_found - 1, 48, "%10ld", (cur_network_metrics.if_network[i].rx_errors - prev_network_metrics.if_network[i].rx_errors) / refresh_second);
                        mvwprintw(main_window, init_if_name_row + if_name_found - 1, 63, "%10ld", (cur_network_metrics.if_network[i].rx_dropped - prev_network_metrics.if_network[i].rx_dropped) / refresh_second);
                        mvwprintw(main_window, init_if_name_row + if_name_found - 1, 77, "%10ld", (cur_network_metrics.if_network[i].tx_packets - prev_network_metrics.if_network[i].tx_packets) / refresh_second);
                        mvwprintw(main_window, init_if_name_row + if_name_found - 1, 89, "%10ld", (cur_network_metrics.if_network[i].tx_bytes - prev_network_metrics.if_network[i].tx_bytes) / 1024 / refresh_second);
                        mvwprintw(main_window, init_if_name_row + if_name_found - 1, 102, "%10ld", (cur_network_metrics.if_network[i].tx_errors - prev_network_metrics.if_network[i].tx_errors) / refresh_second);
                        mvwprintw(main_window, init_if_name_row + if_name_found - 1, 117, "%10ld", (cur_network_metrics.if_network[i].tx_dropped - prev_network_metrics.if_network[i].tx_dropped) / refresh_second);
                        mvwprintw(main_window, init_if_name_row + if_name_found - 1, 129, "%10ld", (cur_network_metrics.if_network[i].collisions - prev_network_metrics.if_network[i].collisions) / refresh_second);

                        break;
                    }
                }
            }

            /* disk metrics*/
            for (int i = 0; i < ret_get_disk_metrics; ++i) {
                for (int j = 0; j < prev_ret_get_disk_metrics; ++j) {
                    /* only process if current disk name exists */
                    if (strcmp(cur_disk_metrics.diskstats[i].disk_name, prev_disk_metrics.diskstats[i].disk_name) == 0) {
                        ++disk_name_found;

                        print_delimiter(main_window, init_if_name_row + if_name_found + disk_name_found + 6 - 1, disk_column_positions, SIZEOF(disk_column_positions));

                        /* print disk metrics */
                        mvwprintw(main_window, init_if_name_row + if_name_found + disk_name_found + 6 - 1, 1, "%-12s", cur_disk_metrics.diskstats[i].disk_name);
                        mvwprintw(main_window, init_if_name_row + if_name_found + disk_name_found + 6 - 1, 17, "%12ld", (cur_disk_metrics.diskstats[i].reads - prev_disk_metrics.diskstats[i].reads) / refresh_second);
                        mvwprintw(main_window, init_if_name_row + if_name_found + disk_name_found + 6 - 1, 31, "%15ld", ((cur_disk_metrics.diskstats[i].sector_read - prev_disk_metrics.diskstats[i].sector_read)) * cur_disk_metrics.diskstats[i].sector_size / 1024 / refresh_second);
                        mvwprintw(main_window, init_if_name_row + if_name_found + disk_name_found + 6 - 1, 50, "%12ld", (cur_disk_metrics.diskstats[i].writes - prev_disk_metrics.diskstats[i].writes) / refresh_second);
                        mvwprintw(main_window, init_if_name_row + if_name_found + disk_name_found + 6 - 1, 64, "%15ld", ((cur_disk_metrics.diskstats[i].sector_write - prev_disk_metrics.diskstats[i].sector_write)) * cur_disk_metrics.diskstats[i].sector_size / 1024 / refresh_second);

                        break;
                    }
                }
            }
        }

        wrefresh(main_window);

        /* sleep */
        napms((int)refresh_second * 1000);

        /* copy the current metrics as previous ones for next calculation */
        prev_os_metrics = cur_os_metrics;
        prev_memory_metrics = cur_memory_metrics;
        prev_network_metrics = cur_network_metrics;
        prev_disk_metrics = cur_disk_metrics;

        prev_ret_get_interface_metrics = ret_get_interface_metrics;
        prev_ret_get_disk_metrics = ret_get_disk_metrics;

        /* set flag once previous data is copied */
        prev_data_flag = 1;
    }

    /* terminate ncurses window */
    wstandend(main_window);
    delwin(main_window);
    endwin();

    /* print error message if error_flag is set */
    if (error_flag > 0) {
        fprintf(stderr, "%s\n", error_msg);
        exit_code = EXIT_FAILURE;
    }

    exit(exit_code);
}
