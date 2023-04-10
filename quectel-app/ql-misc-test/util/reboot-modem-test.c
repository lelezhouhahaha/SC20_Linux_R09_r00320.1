/*
 *      Copyright:  (C) 2019 quectel
 *                  All rights reserved.
 *
 *       Filename:  reboot-modem-test.c
 *    Description:  This file used to test API of library
 *                 
 *        Version:  1.0.0(2019年10月30日)
 *         Author:  Peeta <peeta.chen@quectel.com>
 *      ChangeLog:  1, Release initial version on "2019年10月30日 20时35分33秒"
 *                 
 */
#include <stdio.h>
#include <ql_misc.h>

int main (int argc, char **argv)
{
    int ret = 0;

    ret = QL_MISC_Reboot_Modem();
    if (ret < 0)
        printf("QL_MISC_reboot_Modem failure with ret = %d\n", ret);

    return 0;
} /* ----- End of main() ----- */

