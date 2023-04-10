/*
 * Copyright: (C) 2014 EAST fulinux <fulinux@sina.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <hexparser.h>

#define HEXFILE_TEST_PATH "/tmp/fileparser.hex"

struct param {
    uint32_t a;
    uint32_t b;
    uint8_t c;
    uint32_t d;
    uint8_t e;
};


/*
 * 主函数. 
 */
int main (int argc, char **argv)
{
    int ret;
    uint32_t filelen;
    struct param *p;
    uint8_t *buf;

    p = (struct param *)malloc(sizeof(struct param));
    if(p == NULL) {
        perror("malloc param");
        return -1;
    }

    memset((void *)p, 0, sizeof(struct param));

    buf = (uint8_t *)p;
    ret = readwholefile(HEXFILE_TEST_PATH, buf, &filelen);
    if(ret == 0){
        printf("a: %d, b: %d, c: %d, d: %d, e: %d\n",
                p->a, p->b, p->c, p->d, p->e);
    }else{
        p->a = 1;
        p->b = 2;
        p->c = 3;
        p->d = 4;
        p->e = 5;
        filelen = sizeof(struct param);
        ret = writefile(HEXFILE_TEST_PATH, (uint8_t *)p, filelen, 0, 0);
        if(ret == 0){
            printf("write file success\n");
        }else{
            printf("write file failure\n");
        }
    }

    free (p);
    p	= NULL;

    return 0;
} /* ----- End of main() ----- */

