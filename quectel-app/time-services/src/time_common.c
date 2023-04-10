/*
 *      Copyright:  (C) 2019 quectel
 *                  All rights reserved.
 *
 *       Filename:  time_daemon.c
 *        Version:  1.0.0(2019年12月04日)
 *         Author:  Peeta <peeta.chen@quectel.com>
 *      ChangeLog:  1, Release initial version on "2019年12月04日 09时53分31秒"
 */
#include "time_common.h"

int ntp_synced(void)
{
    struct timex txc = {};

    if (adjtimex(&txc) < 0) 
        return -1;

    if (txc.status & STA_UNSYNC)
        return -2;

    return 0;
}

int rtc_get(int64_t *msecs)
{
    int rc, fd;
    time_t secs = 0;
    struct tm rtc_tm;

    fd = open("/dev/rtc0", O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Daemon:%s: Unable to read from RTC device\n", __func__);
        goto fail_rtc;
    }

    rc = ioctl(fd, RTC_RD_TIME, &rtc_tm);
    if (rc < 0) {
        printf("Daemon: %s: Unable to read from RTC device\n", __func__);
        goto fail_rtc;
    }

#if 0
    printf("Daemon:%s: Time read from RTC -- MM/DD/YY HH:MM:SS"
            "%d/%d/%d %d:%d:%d\n", __func__, rtc_tm.tm_mon,
            rtc_tm.tm_mday, rtc_tm.tm_year, rtc_tm.tm_hour,
            rtc_tm.tm_min, rtc_tm.tm_sec);
#endif

    /* Convert the time to UTC and then to milliseconds and store it */
    secs = mktime(&rtc_tm);
    secs += rtc_tm.tm_gmtoff;
    if (secs < 0) {
        fprintf(stderr, "Daemon: Invalid RTC seconds = %ld\n", secs);
        goto fail_rtc;
    }

    *msecs = SEC_TO_MSEC(secs);

    close(fd);
    return 0;

fail_rtc:
    close(fd);
    return -EINVAL;
}

int time_persistant_memory_opr(const char *file_name, time_persistant_opr_type rd_wr, int64_t *data_ptr)
{
    char fname[120];
    int fd;

    /* location where offset is to be stored */
    snprintf(fname, 100, "%s/%s", "/data/time", file_name);
    //printf("Daemon:Opening File: %s\n", fname);

    switch(rd_wr) {
        case TIME_READ_MEMORY:
            //printf("Daemon:%s: Read operation\n", __func__);
            if ((fd = open(fname, O_RDONLY)) < 0) {
                printf("Daemon: Unable to open file for read\n");
                goto fail_operation;
            }

            if (read(fd, (int64_t *)data_ptr, sizeof(int64_t)) < 0) {
                fprintf(stderr, "Daemon:%s:Error reading from file\n", __func__);
                close(fd);
                goto fail_operation;
            }

            break;
        case TIME_WRITE_MEMORY:
            //printf("Daemon:%s: write operation\n", __func__);
            if ((fd = open(fname, O_RDWR | O_SYNC)) < 0) {
                printf("Daemon:Unable to open file,creating file\n");
                if (fd = open(fname, O_CREAT | O_RDWR | O_SYNC, 0644) < 0) {
                    fprintf(stderr, "Daemon:%s:Daemon:Unable to create file, exiting\n", __func__);
                    goto fail_operation;
                }
                close(fd);

                if ((fd = open(fname, O_RDWR | O_SYNC)) < 0) {
                    fprintf(stderr, "Daemon:Unable to open file,creating file\n");
                    goto fail_operation;
                }
            }

            if (write(fd, (int64_t *)data_ptr, sizeof(int64_t)) < 0) {
                fprintf(stderr, "Daemon:%s:Daemon:Unable to create file, exiting\n", __func__);

                close(fd);
                goto fail_operation;
            }
            break;
        default:
            return -EINVAL;
    }

    close(fd);
    return 0;

fail_operation:
    return -EINVAL;
}
