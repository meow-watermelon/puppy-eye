#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "disk.h"

long int get_disk_size(char *disk_name) {
    long int disk_size;

    FILE *disk_size_file;
    disk_size_file = NULL;
    char disk_size_file_path[PATH_MAX];
    int ret_snprintf;

    ret_snprintf = snprintf(disk_size_file_path, sizeof(disk_size_file_path), "/sys/block/%s/size", disk_name);
    if (ret_snprintf < 0) {
        goto handle_error;
    }

    disk_size_file = fopen(disk_size_file_path, "r");
    if (disk_size_file == NULL) {
        goto handle_error;
    }

    int ret_fscanf = fscanf(disk_size_file, "%ld", &disk_size);
    if (ret_fscanf < 1 || ret_fscanf == EOF) {
        goto handle_error;
    }

    fclose(disk_size_file);

    return disk_size;

handle_error:
    if (disk_size_file != NULL) {
        fclose(disk_size_file);
    }

    return 0;
}

int get_disk_sector_size(char *disk_name) {
    int disk_sector_size;

    FILE *disk_sector_size_file;
    disk_sector_size_file = NULL;
    char disk_sector_size_file_path[PATH_MAX];
    int ret_snprintf;

    ret_snprintf = snprintf(disk_sector_size_file_path, sizeof(disk_sector_size_file_path), "/sys/block/%s/queue/logical_block_size", disk_name);
    if (ret_snprintf < 0) {
        goto handle_error;
    }

    disk_sector_size_file = fopen(disk_sector_size_file_path, "r");
    if (disk_sector_size_file == NULL) {
        goto handle_error;
    }

    int ret_fscanf = fscanf(disk_sector_size_file, "%d", &disk_sector_size);
    if (ret_fscanf < 1 || ret_fscanf == EOF) {
        goto handle_error;
    }

    fclose(disk_sector_size_file);

    return disk_sector_size;

handle_error:
    if (disk_sector_size_file != NULL) {
        fclose(disk_sector_size_file);
    }

    return DEFAULT_SECTOR_SIZE;
}

int get_disk_metrics(struct disk_metrics *input_disk_metrics) {
    /* disk count */
    int count = 0;

    /* placeholder for unused fields */
    long int unused;

    /* disk metrics variables */
    char disk_name[DISK_NAME_LEN];
    long int reads;
    long int sector_read;
    long int writes;
    long int sector_write;

    FILE *diskstats_file;
    diskstats_file = NULL;

    /* read line buffer variable */
    char line[BUFSIZ];

    /* read /proc/diskstats file */
    diskstats_file = fopen("/proc/diskstats", "r");
    if (diskstats_file == NULL) {
        goto handle_error;
    }

    while (fgets(line, sizeof(line), diskstats_file) != NULL) {
        int ret_sscanf = sscanf(line, " %ld %ld %s %ld %ld %ld %ld %ld %ld %ld %*s",
            &unused, &unused, disk_name, &reads, &unused, &sector_read, &unused, &writes, &unused, &sector_write
        );

        if (ret_sscanf < 5 || ret_sscanf == EOF) {
            fprintf(stderr, "ERROR: unable to parse line %s\n", line);
            continue;
        }

        /* we only fill out disk metrics if disk size > 0 */
        if (get_disk_size(disk_name) > 0) {
            strcpy(input_disk_metrics->diskstats[count].disk_name, disk_name);
            input_disk_metrics->diskstats[count].sector_size = get_disk_sector_size(disk_name);
            input_disk_metrics->diskstats[count].reads = reads;
            input_disk_metrics->diskstats[count].sector_read = sector_read;
            input_disk_metrics->diskstats[count].writes = writes;
            input_disk_metrics->diskstats[count].sector_write = sector_write;

            ++count;
        } else {
            continue;
        }

        /* stop parsing if number of disks is greater than DISK_COUNT */
        if (count >= DISK_COUNT) {
            fprintf(stderr, "ERROR: number of disks is greater than %d\n", DISK_COUNT);
            break;
        }
    }

    /* close file handle */
    fclose(diskstats_file);

    return count;

handle_error:
    if (diskstats_file != NULL) {
        fclose(diskstats_file);
    }

    return -1;
}
