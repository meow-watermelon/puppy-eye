#ifndef DISK_H
#define DISK_H

#define DEFAULT_SECTOR_SIZE 512
#define DISK_COUNT 64
#define DISK_NAME_LEN 32

struct disk {
    char disk_name[DISK_NAME_LEN];
    int sector_size;
    long int reads; /* reads completed successfully */
    long int sector_read; /* sectors read */
    long int writes; /* writes completed successfully */
    long int sector_write; /* sectors written */
};

struct disk_metrics {
    /* disk metrics */
    struct disk diskstats[DISK_COUNT];
};

extern long int get_disk_size(char *disk_name);
extern int get_disk_sector_size(char *disk_name);
extern int get_disk_metrics(struct disk_metrics *input_disk_metrics);

#endif /* DISK_H */
