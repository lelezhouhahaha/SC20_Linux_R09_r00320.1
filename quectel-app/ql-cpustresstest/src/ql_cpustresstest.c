/*********************************************************************************
 *      Copyright:  (C) 2020 quectel
 *                  All rights reserved.
 *
 *       Filename:  ql_cpustresstest.c
 *                 
 *        Version:  1.0.0(2020年04月28日)
 *         Author:  Peeta <peeta.chen@quectel.com>
 *      ChangeLog:  1, Release initial version on "2020年04月28日 09时48分07秒"
 ********************************************************************************/
#include <stdio.h>   /*  Standard input/output definitions */
#include <stdlib.h>
#include <string.h>  /*  String function definitions */
#include <unistd.h>  /*  UNIX standard function definitions */
#include <fcntl.h>   /*  File control definitions */
#include <errno.h>   /*  Error number definitions */
#include <termios.h> /*  POSIX terminal control definitions */
#include <limits.h>
#include <errno.h>
#include <getopt.h>
#include <pthread.h>

static unsigned int threadnumber = 1;
static unsigned int thread_sum = 0;

void *cpu_consume_thread(void)
{
    printf ("cpu_consume_thread is %d\n", ++thread_sum);
    while(1) {
    };
}

static void print_usage(const char *prog)
{
    printf("Usage: %s [-nh]\n", prog);
    puts(   "  -n --threadnumber <number>   Number of thread(default is 1, e.g: -n 2)\n"
            "  -h --help            Help\n");
    exit(1);
}

static void parse_opts(int argc, char *argv[])
{
    while (1) {
        const struct option lopts[] = {
            { "threadnumber",  1, 0, 'n' },
            { "help",  0, 0, 'h' },
            { NULL, 0, 0, 0 },
        };
        int c;

        c = getopt_long(argc, argv, "n:h", lopts, NULL);

        if (c == -1)
            break;

        switch (c) {
            case 'n':
                threadnumber = atoi(optarg);
                break;
            case 'h':
            default:
                print_usage(argv[0]);
                break;
        }
    }
}

int main (int argc, char **argv)
{
    int ret;
    int i = 0;

    parse_opts(argc, argv);

    pthread_t *tid;

    tid = (pthread_t *)malloc(sizeof(pthread_t) * threadnumber);

    for (i = 0; i < threadnumber; i++) {
        ret = pthread_create(&tid[i], NULL, (void *)cpu_consume_thread, NULL);
        printf ("create %d thread ret = %d\n", i, ret);
    }

    while(1) {
        sleep(UINT_MAX);
    }

    return 0;
} /* ----- End of main() ----- */

