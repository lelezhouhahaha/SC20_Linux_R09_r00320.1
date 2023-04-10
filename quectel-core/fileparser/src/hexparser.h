/*
 * created by cwenfu 傅克文.
 * modified by fulinux <fulinux@sina.com>
 * file.c文件：提供系统中对于二进制文件的读写api函数.
 */
#ifndef _HEXPARSER_H_
#define _HEXPARSER_H_

#ifdef __cplusplus
#define BEGIN_C_DECLS   extern "C" {
#define END_C_DECLS }
#else
#define BEGIN_C_DECLS
#define END_C_DECLS
#endif

BEGIN_C_DECLS

#include <stdint.h>

#define FLAG_FHEAD 0x68   /* 文件头标志 */
#define FLAG_FTEXT 0x18   /* 文件内容标志 */
#define FLAG_FEND  0x16   /* 文件结束标志 */
#define LEN_FHEAD 32      /* 文件头大小 */
#define POS_FINISH 2      /* 文件完成标志的位移 */
#define OFFSET_TEXT  32   /* 文件内容位移 */

struct file_format
{
    uint8_t headflag;      /* 文件头标志  0x68 */
    uint8_t headlen;       /* 文件头长度 = 8 byte*/
    uint8_t finish_flag;   /* 文件完成标志 0: 未完成 1: 已完成 */
    uint8_t textflag;      /* 文件内容标志  0x18 */

    uint8_t name[16];      /* 文件名称 */
    uint8_t version[2];    /* 版本信息  如10.01 */
    uint8_t date[6];       /* 时间记录 yymmddhhmmss */ 
    uint32_t textlen;      /* 文件长度 */

    uint8_t checkbyte;     /* 总加和校验位 */
    uint8_t endflag;       /* 文件结束标记 0x16 */
};

/* API */
int readwholefile(char *filename, uint8_t *filetext, uint32_t *filelen);
int readfile(char *filename, uint8_t *filetext, uint32_t fileoffset, uint32_t readlen);
int writefile(char *filename, uint8_t *filetext, uint32_t filelen, uint32_t fileoffset, uint8_t write_type);
int copyfile(char *srcfilename, char *desfilename);

END_C_DECLS

#endif /* _HEXPARSER_H_ */
