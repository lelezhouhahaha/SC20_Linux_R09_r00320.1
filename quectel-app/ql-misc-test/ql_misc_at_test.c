/**
 *   Copyright:  (C) 2021 Quectel
 *               All rights reserved.
 *
 *    Filename:  ql_misc_at_test.c
 * Description:  This file 
 *          
 *     Version:  1.0.0(2021年03月31日)
 *      Author:  Peeta Chen <peeta.chen@quectel.com>
 *   ChangeLog:  1, Release initial version on "2021年03月31日 10时58分34秒"
 *                 
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/**
 *  Description:
 *   Input Args:
 *  Output Args:
 * Return Value:
 */
int main (int argc, char **argv)
{
	int i = 0;
	int ret;
	char cmd[500] = "ati";
	char cmd_resp[1024] = {0};

	for (;;) {
		i++;
		memset(cmd_resp, 0, sizeof(cmd_resp));
		ret = QLMISC_SendAT(cmd, cmd_resp, sizeof(cmd_resp));
		if (ret < 0) {
			printf("QLMISC_SendAT failed with return:%d, times = %d\n", ret, i);
			break;
		}else {
			printf("AT command response[%d][%d]:\n", ret, i);
			printf("%s\n", cmd_resp);
		}

		usleep(500000);
	}

	return 0;
} /* ----- End of main() ----- */

