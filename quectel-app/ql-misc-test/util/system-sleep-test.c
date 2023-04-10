/*
 * Copyright: (C) 2019 Quectel
 *            All rights reserved.
 * Author:  Peeta <peeta.chen@quectel.com>
 */
#include <stdio.h>
#include <ql_misc.h>

int main (int argc, char **argv)
{
    int ret = 0;

    ret = QL_MISC_System_Sleep();
    if (ret < 0)
        printf("QL_MISC_System_Sleep failed: %d\n", ret);

    return 0;
} /* ----- End of main() ----- */

