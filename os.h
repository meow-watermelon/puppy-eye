#ifndef OS_H
#define OS_H

struct os_metrics {
    /* load average */
    double loadavg_1m;
    double loadavg_5m;
    double loadavg_15m;

    /* file descriptors usage */
    long int fd_usage;

    /* process states */
    long int total_process;
    long int running_process;
    long int blocked_process;
    long int zombie_process;

    /* context switches count */
    long int context_switches;

    /* current logged in users count */
    long int current_users;
};

extern int get_loadavg(struct os_metrics *input_os_metrics);
extern int get_fd_usage(struct os_metrics *input_os_metrics);
extern int get_process_states(struct os_metrics *input_os_metrics);
extern void get_current_users(struct os_metrics *input_os_metrics);

#endif /* OS_H */
