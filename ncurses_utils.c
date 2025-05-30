#include <ncurses.h>
#include "ncurses_utils.h"

void construct_window_layout(WINDOW *input_window, int interface_count) {
    /* layout constants */
    char *os_metrics_banner = "OS Metrics";
    char *os_loadavg_layout = "Load Average(1min / 5min / 15min):                         File Descriptor Usage:               Context Switch:            / second";
    char *os_processes_layout = "Processes:          total |          running |          blocked |          zombie";
    char *os_users_layout = "Users:";

    char *memory_metrics_banner = "Memory Metrics";
    char *memory_usage_layout = "Memory Usage(MB):            total |            avail |            free |            buffer |            cache |            page tables";
    char *swap_usage_layout = "Swap Usage(MB):            total |            free";
    char *page_faults_layout = "Page Faults Count:              major faults / second |              minor page faults / second";

    char *network_metrics_banner = "Network Metrics";
    char *arp_cache_entries_layout = "ARP Cache Entries:         ";
    char *interface_io_banner_layout = "[Network Interface I/O (per second)]";
    char *interface_layout = "Interface Name    |  RX Packet  |   RX KB   |  RX Error  |  RX Dropped  |  TX Packet  |   TX KB   |  TX Error  |  TX Dropped  |  Collision";

    char *disk_metrics_banner = "Disk Metrics";
    char *disk_io_banner_layout = "[Disk I/O (per second)]";
    char *disk_layout = "Disk Name    |  Read Count  |     Read KB    |  Write Count  |     Write KB";

    /* move curser and print layout 
     * banner should have A_STANDOUT attribute; metric names should have A_BOLD attribute
    */
    wattron(input_window, A_STANDOUT);
    mvwprintw(input_window, 1, 1, os_metrics_banner);
    mvwprintw(input_window, 8, 1, memory_metrics_banner);
    mvwprintw(input_window, 15, 1, network_metrics_banner);

    /* disk metrics beginning postition is based on the last network interface position(interface_count)*/
    mvwprintw(input_window, 20 + interface_count + 3, 1, disk_metrics_banner);
    wattroff(input_window, A_STANDOUT);

    wattron(input_window, A_BOLD);
    mvwprintw(input_window, 3, 1, os_loadavg_layout);
    mvwprintw(input_window, 4, 1, os_processes_layout);
    mvwprintw(input_window, 5, 1, os_users_layout);
    mvwprintw(input_window, 10, 1, memory_usage_layout);
    mvwprintw(input_window, 11, 1, swap_usage_layout);
    mvwprintw(input_window, 12, 1, page_faults_layout);
    mvwprintw(input_window, 17, 1, arp_cache_entries_layout);
    mvwprintw(input_window, 19, 1, interface_io_banner_layout);
    mvwprintw(input_window, 20, 1, interface_layout);
    mvwprintw(input_window, 20 + interface_count + 5, 1, disk_io_banner_layout);
    mvwprintw(input_window, 20 + interface_count + 6, 1, disk_layout);
    wattroff(input_window, A_BOLD);

    mvwhline(input_window, 7, 1, ACS_HLINE, COLS - 2);
    mvwhline(input_window, 14, 1, ACS_HLINE, COLS - 2);
    mvwhline(input_window, 20 + interface_count + 2, 1, ACS_HLINE, COLS - 2);

    /* print footer */
    mvwprintw(input_window, LINES - 1, 10, "press 'Q' to exit");
    
    /* refresh window */
    wrefresh(input_window);
}

void print_delimiter(WINDOW *input_window, int row_number, int *column_positions, size_t column_size) {
    char *delimiter = "|";

    wattron(input_window, A_BOLD);
    for (size_t i = 0; i < column_size; ++i) {
        mvwprintw(input_window, row_number, column_positions[i], delimiter);
    }
    wattroff(input_window, A_BOLD);
}
