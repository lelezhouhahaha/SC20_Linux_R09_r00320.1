/**
 *   Copyright:  (C) 2020 Quectel
 *               All rights reserved.
 *    Filename:  send-at-tool.c
 * Description:  QL_MISC_SendAT() API example
 *     Version:  1.0.0(2020年09月17日)
 *      Author:  Peeta Chen <peeta.chen@quectel.com>
 *   ChangeLog:  1, Release initial version on "2020年09月17日 16时00分18秒"
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ql_misc.h>

static int getInputString(char *str, size_t size)
{
	int i = 0;
	int ret = 0;

	if (str == NULL || size <= 0) {
		return -1;
	}

	memset(str, 0, size);
	if (fgets(str, size, stdin) == NULL)
		return 0;

	for (i = 0; i < size; i++) {
		if (str[i] == '\n') {
			str[i] = '\0';
			break;
		}
	}

	ret = strlen(str);

	return ret;
}

int main (int argc, char **argv)
{
	int ret = 0;
	char cmd[520] = {0};
	char cmd_resp[1024] = {0};

	for (;;) {
		printf("Please input AT command(-1: exit):");
		ret = getInputString(cmd, sizeof(cmd));
		if (ret < 2)
			continue;

		if (strstr(cmd, "-1") != NULL){
			break;
		}

		memset(cmd_resp, 0, sizeof(cmd_resp));
		ret = QL_MISC_SendAT(cmd, cmd_resp, sizeof(cmd_resp));
		if (ret < 0)
			printf ("QL_MISC_SendAT failed with return:%d\n", ret);
		else {
			printf ("AT command response[%d]:\n", ret);
			printf ("%s\n", cmd_resp);
		}
	}

	return 0;
} /* ----- End of main() ----- */
