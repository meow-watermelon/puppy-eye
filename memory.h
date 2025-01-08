#ifndef MEMORY_H
#define MEMORY_H

struct memory_metrics {
    /* memory usage */
    long int total_memory; /* unit: kB */
    long int free_memory; /* unit: kB */
    long int avail_memory; /* unit: kB */
    long int buffer; /* unit: kB */
    long int cache; /* unit: kB */
    long int page_tables; /* unit: kB */

    /* swap usage */
    long int total_swap; /* unit: kB */
    long int free_swap; /* unit: kB */

    /* page faults count*/
    long int total_page_faults;
    long int major_page_faults;
    long int minor_page_faults;
};

struct meminfo_mappings {
    char *meminfo_key;
    long int *memory_metric_ptr;
};

extern int get_memory_usage(struct memory_metrics *input_memory_metrics);

#endif /* MEMORY_H */
