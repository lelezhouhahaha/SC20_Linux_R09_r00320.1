/*
 * Copyright:  (C) 2020 quectel
 *             All rights reserved.
 *  Filename:  time_sync.c
 *   Version:  1.0.0(2020年07月20日)
 *    Author:  Peeta <peeta.chen@quectel.com>
 */                 
#include <stdio.h>
#include <getopt.h>

#include "time_common.h"

static char *dateTime = NULL;
static int setTime = 0;

static void print_usage(const char *prog)
{
    printf("Usage: %s [-s TIME] (e.g: %s -s \"2020-7-20 20:10:45\")\n", prog, prog);
    puts(   "  -s --set  TIME  set system time\n");

    exit(1);
}

static void parse_opts(int argc, char *argv[])
{
    while (1) {
        const struct option lopts[] = {
            { "set",    1, 0, 's' },
            { NULL, 0, 0, 0 },
        };

        int c;
        c = getopt_long(argc, argv, "s:", lopts, NULL);
        if (c == -1)
            break;
        switch (c) {
            case 's':
                setTime = 1;
                dateTime = optarg;
                break;
            default:
                print_usage(argv[0]);
                break;
        }
    }
}


int main (int argc, char **argv)
{
    int64_t rtc_msecs;
    int64_t system_msecs;
    int64_t offset_ms = 0;
    struct timeval stime;
    char cmd[100] = {0};

    parse_opts(argc, argv);
    if (setTime == 0)
        print_usage(argv[0]);

    if (ntp_synced() == 0) {
        printf ("The time has been synchronised\n");
        return 0;
    }

    if (setTime && dateTime && strlen(dateTime) > 0) {
        snprintf(cmd, sizeof(cmd), "date -s \"%s\"", dateTime);
        printf ("%s\n", cmd);
        if (system(cmd) != 0) {
            perror("system date command");
            return -2;
        }
    }

    if (rtc_get(&rtc_msecs) != 0) {
        printf("Get RTC time failed\n");
        return -1;
    }

    gettimeofday(&stime, NULL);

    system_msecs = SEC_TO_MSEC(stime.tv_sec);
    //printf ("system_msecs = %llxms\n", system_msecs);

    system_msecs += USEC_TO_MSEC(stime.tv_usec);
    //printf ("system_msecs = %llxms\n", system_msecs);

    offset_ms = system_msecs - rtc_msecs;
    //printf ("offset_ms = %llxms\n", offset_ms);
    if (time_persistant_memory_opr(DELTA_TIME_FILE,
                TIME_WRITE_MEMORY, &offset_ms) != 0) {
        printf("Sync time failed\n");
        return -2;
    }

    printf("Sync time success!\n");

    return 0;
} /* ----- End of main() ----- */
