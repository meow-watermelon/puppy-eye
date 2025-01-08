#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "memory.h"

int get_memory_usage(struct memory_metrics *input_memory_metrics) {
    /* initialize variables for memory metrics */
    input_memory_metrics->total_memory = 0;
    input_memory_metrics->free_memory = 0;
    input_memory_metrics->avail_memory = 0;
    input_memory_metrics->buffer = 0;
    input_memory_metrics->cache = 0;
    input_memory_metrics->page_tables = 0;
    input_memory_metrics->total_swap = 0;
    input_memory_metrics->free_swap = 0;
    input_memory_metrics->total_page_faults = 0;
    input_memory_metrics->major_page_faults = 0;
    input_memory_metrics->minor_page_faults = 0;

    FILE *meminfo_file;
    meminfo_file = NULL;

    FILE *vmstat_file;
    vmstat_file = NULL;

    /* read line buffer variable */
    char line[BUFSIZ];

    /* metric mapping struct size */
    size_t mappings_size;

    /* construct metric names to memory_metrics pointers */
    struct meminfo_mappings meminfo_mappings_name[] = {
        {"MemTotal", &input_memory_metrics->total_memory},
        {"MemFree", &input_memory_metrics->free_memory},
        {"MemAvailable", &input_memory_metrics->avail_memory},
        {"Buffers", &input_memory_metrics->buffer},
        {"Cached", &input_memory_metrics->cache},
        {"PageTables", &input_memory_metrics->page_tables},
        {"SwapTotal", &input_memory_metrics->total_swap},
        {"SwapFree", &input_memory_metrics->free_swap},
    };
    mappings_size = sizeof(meminfo_mappings_name) / sizeof(meminfo_mappings_name[0]);

    /* read /proc/meminfo file */
    meminfo_file = fopen("/proc/meminfo", "r");
    if (meminfo_file == NULL) {
        goto handle_error;
    }

    while (fgets(line, sizeof(line), meminfo_file) != NULL) {
        for (size_t i = 0; i < mappings_size; ++i) {
            if (strstr(line, meminfo_mappings_name[i].meminfo_key) != NULL) {
                /* check if metric name is exact match, DO NOT skip this step */
                size_t metric_name_length = strlen(meminfo_mappings_name[i].meminfo_key);

                if (strncmp(line, meminfo_mappings_name[i].meminfo_key, metric_name_length) == 0) {
                    int ret_sscanf = sscanf(line, "%*s %ld kB", meminfo_mappings_name[i].memory_metric_ptr);

                    if (ret_sscanf < 1 || ret_sscanf == EOF) {
                        fprintf(stderr, "ERROR: failed to parse %s line\n", meminfo_mappings_name[i].meminfo_key);
                    }
                    break;
                }
            }
        }
    }

    /* close file handle */
    fclose(meminfo_file);

    /* construct metric names to memory_metrics pointers */
    struct meminfo_mappings vmstat_mappings_name[] = {
        {"pgfault", &input_memory_metrics->total_page_faults},
        {"pgmajfault", &input_memory_metrics->major_page_faults},
    };
    mappings_size = sizeof(vmstat_mappings_name) / sizeof(vmstat_mappings_name[0]);

    /* read /proc/vmstat file */
    vmstat_file = fopen("/proc/vmstat", "r");
    if (vmstat_file == NULL) {
        goto handle_error;
    }

    while (fgets(line, sizeof(line), vmstat_file) != NULL) {
        for (size_t i = 0; i < mappings_size; ++i) {
            if (strstr(line, vmstat_mappings_name[i].meminfo_key) != NULL) {
                /* check if metric name is exact match, DO NOT skip this step */
                size_t metric_name_length = strlen(vmstat_mappings_name[i].meminfo_key);

                if (strncmp(line, vmstat_mappings_name[i].meminfo_key, metric_name_length) == 0) {
                    int ret_sscanf = sscanf(line, "%*s %ld", vmstat_mappings_name[i].memory_metric_ptr);

                    if (ret_sscanf < 1 || ret_sscanf == EOF) {
                        fprintf(stderr, "ERROR: failed to parse %s line\n", vmstat_mappings_name[i].meminfo_key);
                    }
                    break;
                }
            }
        }
    }

    /* calculate minor page faults count*/
    input_memory_metrics->minor_page_faults = input_memory_metrics->total_page_faults - input_memory_metrics->major_page_faults;

    /* close file handle */
    fclose(vmstat_file);

    return 0;

handle_error:
    if (meminfo_file != NULL) {
        fclose(meminfo_file);
    }

    if (vmstat_file != NULL) {
        fclose(vmstat_file);
    }

    return -1;
}
