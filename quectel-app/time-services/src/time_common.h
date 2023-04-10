/*
 * Copyright:  (C) 2019 quectel, all rights reserved.
 *  Filename:  time_daemon.c
 *   Version:  1.0.0(2019.12.04)
 *    Author:  Peeta <peeta.chen@quectel.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/timex.h>
#include <sys/un.h>
#include <linux/rtc.h>
#include <errno.h>
#include <signal.h>

#define DELTA_TIME_FILE "ats_1"

#define SEC_TO_MSEC(s)  ((s) * 1000ULL)
#define MSEC_TO_SEC(s)  ((s) / 1000ULL)
#define USEC_TO_MSEC(s) ((s) / 1000ULL)
#define NSEC_TO_MSEC(s) ((s) / 1000000ULL)

typedef enum time_persistant_operation {
    TIME_READ_MEMORY,
    TIME_WRITE_MEMORY
}time_persistant_opr_type;

int ntp_synced(void);

int rtc_get(int64_t *msecs);

int time_persistant_memory_opr(const char *file_name, time_persistant_opr_type rd_wr, int64_t *data_ptr);
