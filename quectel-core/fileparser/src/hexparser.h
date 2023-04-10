/*
 * created by cwenfu ������.
 * modified by fulinux <fulinux@sina.com>
 * file.c�ļ����ṩϵͳ�ж��ڶ������ļ��Ķ�дapi����.
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

#define FLAG_FHEAD 0x68   /* �ļ�ͷ��־ */
#define FLAG_FTEXT 0x18   /* �ļ����ݱ�־ */
#define FLAG_FEND  0x16   /* �ļ�������־ */
#define LEN_FHEAD 32      /* �ļ�ͷ��С */
#define POS_FINISH 2      /* �ļ���ɱ�־��λ�� */
#define OFFSET_TEXT  32   /* �ļ�����λ�� */

struct file_format
{
    uint8_t headflag;      /* �ļ�ͷ��־  0x68 */
    uint8_t headlen;       /* �ļ�ͷ���� = 8 byte*/
    uint8_t finish_flag;   /* �ļ���ɱ�־ 0: δ��� 1: ����� */
    uint8_t textflag;      /* �ļ����ݱ�־  0x18 */

    uint8_t name[16];      /* �ļ����� */
    uint8_t version[2];    /* �汾��Ϣ  ��10.01 */
    uint8_t date[6];       /* ʱ���¼ yymmddhhmmss */ 
    uint32_t textlen;      /* �ļ����� */

    uint8_t checkbyte;     /* �ܼӺ�У��λ */
    uint8_t endflag;       /* �ļ�������� 0x16 */
};

/* API */
int readwholefile(char *filename, uint8_t *filetext, uint32_t *filelen);
int readfile(char *filename, uint8_t *filetext, uint32_t fileoffset, uint32_t readlen);
int writefile(char *filename, uint8_t *filetext, uint32_t filelen, uint32_t fileoffset, uint8_t write_type);
int copyfile(char *srcfilename, char *desfilename);

END_C_DECLS

#endif /* _HEXPARSER_H_ */
