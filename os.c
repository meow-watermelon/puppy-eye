#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <utmpx.h>
#include "os.h"

int get_loadavg(struct os_metrics *input_os_metrics) {
    /* initialize variables for loadavg */
    input_os_metrics->loadavg_1m = 0.0;
    input_os_metrics->loadavg_5m = 0.0;
    input_os_metrics->loadavg_15m = 0.0;

    FILE *loadavg_file;
    loadavg_file = NULL;

    /* read loadavg file */
    loadavg_file = fopen("/proc/loadavg", "r");
    if (loadavg_file == NULL) {
        goto handle_error;
    }

    int ret_fscanf = fscanf(loadavg_file, "%lf %lf %lf %*s %*s", &input_os_metrics->loadavg_1m, &input_os_metrics->loadavg_5m, &input_os_metrics->loadavg_15m);

    if (ret_fscanf < 3 || ret_fscanf == EOF) {
        fprintf(stderr, "ERROR: failed to parse /proc/loadavg file\n");
    }

    /* close file handle */
    fclose(loadavg_file);

    return 0;

/* error handling routine */
handle_error:
    if (loadavg_file != NULL) {
        fclose(loadavg_file);
    }

    return -1;
}

int get_fd_usage(struct os_metrics *input_os_metrics) {
    /* initialize variable for file descriptors usage */
    input_os_metrics->fd_usage = 0;

    FILE *fd_usage_file;
    fd_usage_file = NULL;

    /* read fd usage file */
    fd_usage_file = fopen("/proc/sys/fs/file-nr", "r");
    if (fd_usage_file == NULL) {
        goto handle_error;
    }

    int ret_fscanf = fscanf(fd_usage_file, "%ld %*s %*s", &input_os_metrics->fd_usage);

    if (ret_fscanf < 1 || ret_fscanf == EOF) {
        fprintf(stderr, "ERROR: failed to parse /proc/sys/fs/file-nr file\n");
    }

    /* close file handle */
    fclose(fd_usage_file);

    return 0;

/* error handling routine */
handle_error:
    if (fd_usage_file != NULL) {
        fclose(fd_usage_file);
    }

    return -1;
}

int get_process_states(struct os_metrics *input_os_metrics) {
    /* initialize variables for all process states + context switches */
    input_os_metrics->total_process = 0;
    input_os_metrics->running_process = 0;
    input_os_metrics->blocked_process = 0;
    input_os_metrics->zombie_process = 0;
    input_os_metrics->context_switches = 0;

    /* initialize variables for running / blocked processes + context switches */
    FILE *stat_file;
    stat_file = NULL;

    /* initialize variables for total processes */
    DIR *proc_dir;
    proc_dir = NULL;
    struct dirent *entry;

    /* initialize variables for zombie processes */
    FILE *proc_pid_state_file;
    proc_pid_state_file = NULL;
    char proc_pid_path[PATH_MAX];
    int ret_snprintf;
    char proc_state;

    /* read line buffer variable */
    char line[BUFSIZ];

    /* read stat file */
    stat_file = fopen("/proc/stat", "r");
    if (stat_file == NULL) {
        goto handle_error;
    }

    while (fgets(line, sizeof(line), stat_file) != NULL) {
        int ret_sscanf;

        /* retrieve number of running processes */
        if (strstr(line, "procs_running") != NULL) {
            ret_sscanf = sscanf(line, "procs_running %ld", &input_os_metrics->running_process);

            if (ret_sscanf < 1 || ret_sscanf == EOF) {
                fprintf(stderr, "ERROR: unable to parse procs_running line\n");
            }
        }

        /* retrieve number of blocked processes */
        if (strstr(line, "procs_blocked") != NULL) {
            ret_sscanf = sscanf(line, "procs_blocked %ld", &input_os_metrics->blocked_process);

            if (ret_sscanf < 1 || ret_sscanf == EOF) {
                fprintf(stderr, "ERROR: unable to parse procs_blocked line\n");
            }
        }

        /* retrieve count of context switches */
        if (strstr(line, "ctxt") != NULL) {
            ret_sscanf = sscanf(line, "ctxt %ld", &input_os_metrics->context_switches);

            if (ret_sscanf < 1 || ret_sscanf == EOF) {
                fprintf(stderr, "ERROR: unable to parse ctxt line\n");
            }
        }
    }

    /* close file handle */
    fclose(stat_file);

    /* retrieve total process state */
    proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        goto handle_error;
    }

    while ((entry = readdir(proc_dir)) != NULL) {
        /* check if the 1st character is digit*/
        if (isdigit(entry->d_name[0])) {
            ++input_os_metrics->total_process;

            /* retrieve zombie process state */
            ret_snprintf = snprintf(proc_pid_path, sizeof(proc_pid_path), "/proc/%s/stat", entry->d_name);
            if (ret_snprintf < 0) {
                continue;
            }

            proc_pid_state_file = fopen(proc_pid_path, "r");
            if (proc_pid_state_file == NULL) {
                continue;
            }

            int ret_fscanf = fscanf(proc_pid_state_file, "%*d %*s %c", &proc_state);

            if (ret_fscanf == 1 && proc_state == 'Z') {
                ++input_os_metrics->zombie_process;
            }

            fclose(proc_pid_state_file);
        }
    }

    /* close directory handle*/
    closedir(proc_dir);

    return 0;

/* error handling routine */
handle_error:
    if (stat_file != NULL) {
        fclose(stat_file);
    }

    if (proc_dir != NULL) {
        closedir(proc_dir);
    }

    return -1;
}

void get_current_users(struct os_metrics *input_os_metrics) {
    /* initialize variable for number of current logged in users */
    input_os_metrics->current_users = 0;

    /* define utmpx pointer */
    struct utmpx *input_utmpx;

    /* rewind pointer to beginning of utmp file, DO NOT skip this step */
    setutxent();

    while ((input_utmpx = getutxent()) != NULL) {
        /* check users who are established and authenticated */
        if ((input_utmpx->ut_type == USER_PROCESS || input_utmpx->ut_type == LOGIN_PROCESS) && input_utmpx->ut_user[0] != '\0') {
            ++input_os_metrics->current_users;
        }
    }

    /* close utmp file*/
    endutxent();
}
