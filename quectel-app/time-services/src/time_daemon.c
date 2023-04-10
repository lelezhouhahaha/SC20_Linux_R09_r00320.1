/*
 *      Copyright:  (C) 2019 quectel
 *                  All rights reserved.
 *
 *       Filename:  time_daemon.c
 *        Version:  1.0.0(2019年12月04日)
 *         Author:  Peeta <peeta.chen@quectel.com>
 *      ChangeLog:  1, Release initial version on "2019年12月04日 09时53分31秒"
 */


#include <stdio.h>
#include "time_common.h"

#define TIME_STATUS_DO_NOTHING   0
#define TIME_STATUS_STORE_ANYHOW   1

int main (int argc, char **argv)
{
    int ret;
    int while_times = 1;
    int status = TIME_STATUS_DO_NOTHING;
    int64_t rtc_msecs;
    int64_t system_msecs;
    int64_t offset_ms = 0;
    int64_t last_offset_ms = 0;
    int64_t delta_ms = 0;
    struct timeval stime;

    while (1) {
        if ((ret = rtc_get(&rtc_msecs)) != 0)
            return ret;

        if (ntp_synced() == 0 || status == TIME_STATUS_STORE_ANYHOW) {
            gettimeofday(&stime, NULL);

            system_msecs = SEC_TO_MSEC(stime.tv_sec);
            //printf ("system_msecs = %llxms\n", system_msecs);

            system_msecs += USEC_TO_MSEC(stime.tv_usec);
            //printf ("system_msecs = %llxms\n", system_msecs);

            offset_ms = system_msecs - rtc_msecs;
            //printf ("offset_ms = %llxms\n", offset_ms);
            if (MSEC_TO_SEC(offset_ms) != MSEC_TO_SEC(last_offset_ms)) {
                time_persistant_memory_opr(DELTA_TIME_FILE, TIME_WRITE_MEMORY, &offset_ms);
                printf("Stored system time.\n");
                last_offset_ms = offset_ms;
            }

            status = TIME_STATUS_DO_NOTHING;
        }

        ret = time_persistant_memory_opr(DELTA_TIME_FILE, TIME_READ_MEMORY, &offset_ms);
        if (ret != 0) {
            last_offset_ms = 0;
            status = TIME_STATUS_STORE_ANYHOW;
            continue;
        }

        gettimeofday(&stime, NULL);

        system_msecs = SEC_TO_MSEC(stime.tv_sec);
        //printf ("system_msecs = %llxms\n", system_msecs);

        system_msecs += USEC_TO_MSEC(stime.tv_usec);
        //printf ("system_msecs = %llxms\n", system_msecs);

        delta_ms = system_msecs - rtc_msecs;
        if (MSEC_TO_SEC(delta_ms - offset_ms) != 0) {
            //printf ("offset_ms = %llxms\n", offset_ms);

            system_msecs = rtc_msecs + offset_ms;

            stime.tv_sec = MSEC_TO_SEC(system_msecs);
            //printf ("stime.tv_sec = %llxms\n", stime.tv_sec);

            stime.tv_usec = (system_msecs % 1000ULL) * 1000ULL;
            //printf ("stime.tv_usec = %llxms\n", stime.tv_usec);

            settimeofday(&stime,NULL);
            printf("Updated system time.\n");
        }

        sleep(while_times);

        if (ntp_synced() != 0)
            while_times = 3;
        else
            while_times *= 2;
    }

    return 0;
} /* ----- End of main() ----- */
