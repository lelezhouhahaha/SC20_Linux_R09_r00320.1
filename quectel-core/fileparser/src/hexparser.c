/*
 * created by cwenfu 傅克文.
 * modified by fulinux <fulinux@sina.com>
 * file.c文件：提供系统中对于二进制文件的读写api函数.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "hexparser.h"

/*
 * writefileflag: 写文件完成标志
 * input:
 *    fp: 文件指针   
 *    flag: 写入的标志
 * return:
 *    -1: 错误      0: 正确
 */
static int writefileflag(FILE *fp, uint8_t flag)
{
    int nwrite;

    /* 将标志写入文件 */
    fseek(fp,  POS_FINISH * sizeof(uint8_t), SEEK_SET);

    nwrite = fwrite(&flag, 1, sizeof(uint8_t), fp);
    if (nwrite !=  sizeof(uint8_t)){
        printf("writefile: write finish flag error\n");
        return -1;
    }

    return 0;
}

/*
 * chksum - 计算和校验
 * 计算和校验前应确保f_apacket不为空，
 * 而且sizeof(f_apacket) >= f_nf_nlen
 */
uint8_t chksum(uint8_t *f_apacket, uint32_t f_nlen)
{
    uint32_t i;
    uint8_t sum = 0;

    for(i = 0; i < f_nlen; i ++){
        sum += f_apacket[i];
    }
    return sum;
}

void lock_set(int fd, int type)
{
    struct flock lock;

    /*赋值lock结构体，加锁整个文件*/
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    while(1){
        lock.l_type = type;

        //根据不同的type来给文件加锁或解锁，如果成功，则返回0，失败则返回-1。
        //举例：如果一个文件原来已经建立了互斥锁，那么再调用fcntl建立锁就会失败，返回-1。
        if((fcntl(fd, F_SETLK, &lock)) == 0) {
            if(lock.l_type == F_RDLCK){			   //如果是共享锁
                //printf("read only set by %d\n", getpid());
            }else if(lock.l_type == F_WRLCK){   //如果是互斥锁
                //printf("write lock set by %d\n", getpid());
            }else if(lock.l_type == F_UNLCK){
                //printf("release lock by %d\n", getpid());
            }

            return;
        }else{
            //获得lock的描述，也就是将文件fd的加锁信息存入到lock结构中,如果成功则返回0,如果失败则返回-1
            if((fcntl(fd, F_GETLK, &lock)) == 0){
                if(lock.l_type != F_UNLCK){
                    if(lock.l_type == F_RDLCK){
                        //printf("read lock already set by %d\n", lock.l_pid);
                    }else if(lock.l_type == F_WRLCK){
                        //printf("write lock already set by %d\n", lock.l_pid);
                    }
                    sleep(1);//
                }
            }else{
                printf("cannot get the description of struck flock.\n");
            }
        }
    }
}

/*
 * readFileInfo: 读取并检查文件头正确性
 * inputs:
 *      file *fp: 文件指针
 *      struct file_format *file: 文件信息结构体
 *      uint8_t *textbuf: 读入文件缓冲 
 *          NULL: 不读 
 *          readlen: 0: 文件的全部内容 其余: 所要读取的长度
 * reutrns;
 *      -1: 错误   0: 正确
 */
static int readfileinfo(FILE *fp, struct file_format *file, uint8_t *textbuf, uint32_t fileoffset, uint32_t readlen)
{
    uint8_t check;
    uint32_t nread;
    uint8_t *pmem = NULL;

    if (NULL == fp){
        printf("readfileinfo: fp is NULL\n");
        return -1;
    }

    rewind(fp);

    /* 检查文件头标志 */
    nread = fread(&file->headflag, 1, sizeof(uint8_t), fp);

    if (FLAG_FHEAD != file->headflag){
        printf("readfileinfo: headflag error\n");
        return -1;
    }

    /* 检查头文件长度 */
    nread = fread(&file->headlen, 1, sizeof(uint8_t), fp);
    if (LEN_FHEAD != file->headlen){
        printf("readfileinfo: head len error\n");
        return -1;
    }

    /* 检查文件完成标志 */
    nread = fread(&file->finish_flag, 1, sizeof(uint8_t), fp);
    if (1 != file->finish_flag){
        printf("readfileinfo: finish flag error\n");
        return -1;
    }

    /* 检查文件内容开始标志 */
    nread = fread(&file->textflag, 1, sizeof(uint8_t), fp);
    if (FLAG_FTEXT != file->textflag){
        printf("readfileinfo: textflag error\n");
        return -1;
    }

    /* 读取文件名称，版本，修改日期 */
    nread = fread(&file->name, 1, 24*sizeof(uint8_t), fp);
    if (nread != 24*sizeof(uint8_t)){
        printf("readfileinfo: name error\n");
        return -1;
    }


    /* 读取文件长度 */
    nread = fread(&file->textlen, 1, sizeof(uint32_t), fp);

    if (sizeof(uint32_t) != nread){
        printf("readfileinfo: read textlen error\n");
        return -1;
    }

    /* 读取文件校验码 */
    fseek(fp, file->textlen, SEEK_CUR);
    nread = fread(&file->checkbyte, 1, sizeof(uint8_t), fp);
    file->checkbyte++; /* 完成标志变化0->1 所以要+1 */
    if (sizeof(uint8_t) != nread){
        printf("readfileinfo: read checksum error\n");
        return -1;
    }

    /* 确认校验和的正确性 */
    pmem = malloc(file->textlen * sizeof(uint8_t));
    if (NULL == pmem){
        printf("readfileinfo: malloc error\n");
        return -1;
    }

    fseek(fp, OFFSET_TEXT, SEEK_SET);
    nread = fread(pmem, 1, file->textlen, fp);
    if (nread != file->textlen){
        printf("readfileinfo: read text error\n");
        free(pmem);
        return -1;
    }

    /* 计算校验和 */
    check = chksum(pmem, file->textlen) + chksum((uint8_t *)file, OFFSET_TEXT);

    if (file->checkbyte != check){  
        printf("readfileinfo: check sum error!!check=%x\n file->checkbyte=%x",check,file->checkbyte);
        free(pmem);
        return -1;
    }

    /* 读文件结束标志 */
    fseek(fp, sizeof(uint8_t), SEEK_CUR);
    nread = fread(&file->endflag, 1, sizeof(uint8_t), fp);
    if (FLAG_FEND != file->endflag){
        printf("readfileinfo: end flag error\n");
        free(pmem);
        return -1;
    }

    /* 读取文件 */
    if (NULL != textbuf){
        if (0 == readlen){  /* 读取文件的全部内容 */
            memcpy(textbuf, pmem, file->textlen);
        }else{  /* 读取文件的部分内容 */
            if (fileoffset + readlen > file->textlen){
                printf("readfileinfo: read text len error\n");
                free(pmem);
                return -1;
            }
            memcpy(textbuf, (pmem + fileoffset), readlen);
        }
    }

    free(pmem);
    return 0;
}

/*
 *readwholefile: 读取整个文件内容
 *
 *inputs:
 *   char *filename: 文件名
 *   uint8_t *filetext: 输出的文件内容
 *   uint32_t *filelen: 文件长度
 *
 *reutrns;
 *   -1: 错误   0: 正确
 */
int readwholefile(char *filename, uint8_t *filetext, uint32_t *filelen)
{
    int ret;
    FILE *fp;
    int fd;
    struct file_format file;
    char name[100];

    sprintf(name, "%s", filename);
    fp = fopen(name, "rb+");
    if (NULL == fp){
        perror("readwholefile fopen");
        return -1;
    }

    /*给文件加读取锁*/
    fd = fileno(fp);
    lock_set(fd, F_RDLCK);
    ret = readfileinfo(fp, &file, filetext, 0, 0);
    if (-1 == ret){
        printf("readwholefile: readfileinfo error\n");
        lock_set(fd, F_UNLCK); /* 给文件解锁 */
        fclose(fp);
        return -1;
    }

    if(filelen != NULL)
        *filelen = file.textlen;

    fflush(fp);

    /*给文件解锁*/
    lock_set(fd, F_UNLCK);
    fclose(fp);

    return 0;
}

/*
 * readfile: 读取文件内容
 *
 * inputs:
 *      char *filename: 文件名
 *      uint8_t *filetext: 读取后的文件内容存放位置
 *      uint32_t fileoffset: 读取的位置偏移
 *      uint32_t readlen: 读入长度
 *
 * reutrns;
 *      -1: 错误   0: 正确
 */
int readfile(char *filename, uint8_t *filetext, uint32_t fileoffset, uint32_t readlen)
{
    int ret;
    FILE *fp;
    int fd;
    struct file_format file;
    char name[100];

    sprintf(name, "%s", filename);
    fp = fopen(name, "rb+");
    if (NULL == fp){
        printf("readfile: open file error\n");
        return -1;
    }

    if (0 == readlen){
        printf("readfile: readlen = 0 error\n");
        fclose(fp);
        return -1;
    }

    /*给文件加读取锁*/
    fd = fileno(fp);
    lock_set(fd, F_RDLCK);
    ret = readfileinfo(fp, &file, filetext, fileoffset, readlen);
    if (-1 == ret){
        printf("readfile: readfileinfo error\n");
        lock_set(fd, F_UNLCK); /* 给文件解锁 */
        fclose(fp);
        return -1;
    }
    fflush(fp);

    /*给文件解锁*/
    lock_set(fd, F_UNLCK);
    fclose(fp);

    return 0;
}


/*
 * writefile: 写文件
 * 
 * inputs:
 *      char *filename: 文件名
 *      uint8_t *filetext: 需要写入的内容
 *      uint32_t filelen: 需要写入的长度
 *      uint32_t fileoffset: 写入位置
 *      uint8_t write_type: 写入类型 
 *      0: 写新文件 1: 部分覆盖原有文件  2: 部分替换原有文件
 *      部分覆盖: 即原有文件在写入位置之后的信息全部作废
 *      部分替换: 即原有文件在写入位置之后的信息依然有效
 *      例子: 假设文件内容为 0 1 2 3 4
 *            在2的位置写5 如果部分覆盖则文件变成 0 1 5
 *                         如果部分替换则文件变成 0 1 5 3 4
 *
 * return:
 *      -1: 写文件错误    0: 写文件正常
 */
int writefile(char *filename, uint8_t *filetext, uint32_t filelen, uint32_t fileoffset, uint8_t write_type)
{
    int fd;
    FILE *fp;
    char name[100];
    uint8_t *pmem = NULL;
    uint32_t nwrite, nread;
    struct file_format file;

    sprintf(name, "%s", filename);

    /* 写新文件 */
    if (0 == write_type){
        fp = fopen(name, "wb+");
        if (NULL == fp){
            printf("writefile: open file error\n");
            return -1;
        }

        /*给文件加写入锁*/
        fd = fileno(fp);
        lock_set(fd, F_WRLCK);

        /* 文件头 */
        file.headflag = FLAG_FHEAD;  /* 文件头标志 */
        file.headlen = LEN_FHEAD;    /* 文件头长度 */
        file.finish_flag = 0;  /* 文件未完成 */
        file.textflag = FLAG_FTEXT;   /* 文件内容标志 */

        /* 文件内容 */
        file.textlen = filelen;  /* 文件长度 */
        rewind(fp); /* 将文件指针指向文件头 */
        /* 将文件信息写入文件 */
        nwrite = fwrite(&file, 1, OFFSET_TEXT, fp);
        if (nwrite != OFFSET_TEXT){
            printf("writefile: write file info error\n");
            lock_set(fd, F_UNLCK);//给文件解锁
            fclose(fp);
            return -1;
        }

        /* 将文件内容写入文件 */
        nwrite = fwrite(filetext, 1, filelen, fp);
        if (nwrite != filelen){
            printf("writefile: write file text error\n");
            lock_set(fd, F_UNLCK);//给文件解锁
            fclose(fp);
            return -1;
        }

        /* 文件校验和 */
        file.checkbyte = chksum(filetext, filelen) + chksum((uint8_t *)&file, OFFSET_TEXT);
        file.endflag = FLAG_FEND;
        /* 将文件尾写入文件 */
        nwrite = fwrite(&file.checkbyte, 1, 2 * sizeof(uint8_t), fp);
        if (nwrite != 2 * sizeof(uint8_t)){
            printf("writefile: write file end error\n");
            lock_set(fd, F_UNLCK);//给文件解锁
            fclose(fp);
            return -1;
        }

        /* 将完成标志写入文件 */
        if (-1 == writefileflag(fp, 1)){
            printf("writefile: write file flag error\n");
            lock_set(fd, F_UNLCK);//给文件解锁
            fclose(fp);
            return -1;
        }
        fflush(fp);

        /*给文件解锁*/
        lock_set(fd, F_UNLCK);
        fclose(fp);
        return 0;
    }

    /* 在原有基础上写文件 */
    fp = fopen((void *)name, "rb+");
    if (NULL == fp){
        printf("writefile: open file error\n");
        return -1;
    }

    /*给文件加写入锁*/
    fd = fileno(fp);
    lock_set(fd, F_WRLCK);

    /* 检查文件正确性 */
    if (-1 == readfileinfo(fp, &file, NULL, 0, 0)){
        printf("writefile: file info error\n");
        lock_set(fd, F_UNLCK);
        fclose(fp);
        return -1;
    }

    file.finish_flag = 0;  /* 标志操作未完成 */
    if (1 == write_type){     /* 覆盖 */
        file.textlen = filelen + fileoffset;  /* 文件长度 */
    }else if (2 == write_type){   /* 替换 */
        if (fileoffset + filelen > file.textlen){  /* 替换不能超过原文件大小 */
            printf("writefile: file len error\n");
            lock_set(fd, F_UNLCK);
            fclose(fp);
            return -1;
        }
    }else{
        printf("writefile: no such write_type\n");
        lock_set(fd, F_UNLCK);
        fclose(fp);
        return -1;
    }

    rewind(fp); /* 将文件指针指向文件头 */
    /* 将文件信息写入文件 */
    nwrite = fwrite(&file, 1, OFFSET_TEXT, fp);
    if (nwrite != OFFSET_TEXT){
        printf("writefile: write file info error\n");
        lock_set(fd, F_UNLCK);
        fclose(fp);
        return -1;
    }

    /* 确定写入文件的位置 */
    fseek(fp, OFFSET_TEXT + fileoffset, SEEK_SET);
    /* 写文件 */
    nwrite = fwrite(filetext, 1, filelen, fp);
    if (nwrite != filelen){
        printf("writefile: write file text error\n");
        lock_set(fd, F_UNLCK);
        fclose(fp);
        return -1;
    }

    /* 计算校验和 */
    pmem = malloc(file.textlen + OFFSET_TEXT);
    if (NULL == pmem){
        printf("writefile: malloc error\n");
        lock_set(fd, F_UNLCK);
        fclose(fp);
        return -1;
    }
    rewind(fp);
    nread = fread(pmem, 1, file.textlen + OFFSET_TEXT, fp);
    if ((file.textlen + OFFSET_TEXT) != nread){
        printf("writefile: file read error\n");
        free(pmem);
        lock_set(fd, F_UNLCK);
        fclose(fp);
        return -1;
    }
    file.checkbyte = chksum(pmem, file.textlen + OFFSET_TEXT);
    free(pmem);
    file.endflag = FLAG_FEND;

    /* 写文件尾 */
    fseek(fp, file.textlen + OFFSET_TEXT, SEEK_SET);
    nwrite = fwrite(&file.checkbyte, 1, 2 * sizeof(uint8_t), fp);
    if (nwrite != 2 * sizeof(uint8_t)){
        printf("writefile: write file end error\n");
        lock_set(fd, F_UNLCK);
        fclose(fp);
        return -1;
    }

    /* 将完成标志写入文件 */
    if (-1 == writefileflag(fp, 1)){
        printf("writefile: write file flag error\n");
        lock_set(fd, F_UNLCK);
        fclose(fp);
        return -1;
    }
    fflush(fp);

    /*给文件解锁*/
    lock_set(fd, F_UNLCK);
    fclose(fp);
    return 0;
}

/*
 * copyfile: 拷贝文件
 *
 * inputs:
 *      char *srcfilename: 源文件名
 *      char *desfilename: 目标文件名
 *
 * reutrns;
 *      -1: 错误   0: 正确
 */
int copyfile(char *srcfilename, char *desfilename)
{
    FILE *fp;
    int fd;
    char filename[100];
    uint8_t *pmem = NULL;
    uint32_t textlen, nread, nwrite;

    /* 打开原文件 */
    sprintf(filename, "%s", srcfilename);
    fp = fopen(filename, "rb");
    if (NULL == fp){
        printf("copyfile: open srcfile error\n");
        return -1;
    }

    /*给文件加读取锁*/
    fd = fileno(fp);
    lock_set(fd, F_RDLCK);

    /* 读取文件长度 */
    rewind(fp);
    fseek(fp, 28 * sizeof(uint8_t), SEEK_SET);
    nread = fread(&textlen, 1, sizeof(uint32_t), fp);
    if (sizeof(uint32_t) != nread){
        printf("copyfile: read srcfile textlen error\n");
        lock_set(fd, F_UNLCK);
        fclose(fp);
        return -1;
    }

    textlen += 34; /* add by kaka 20110822 */

    /* 申请内存 */
    pmem = malloc(textlen);
    if (NULL == pmem){
        printf("copyfile: malloc mem fail\n");
        lock_set(fd, F_UNLCK);
        fclose(fp);
        return -1;
    }

    /* 读取源文件内容到内存中 */
    rewind(fp);
    nread = fread(pmem, 1, textlen, fp);
    if (textlen != nread){
        printf("copyfile: read srcfile file error\n");
        free(pmem);
        lock_set(fd, F_UNLCK);
        fclose(fp);
        return -1;
    }

    /* 关闭源文件 */
    fflush(fp);

    /*给文件解锁*/
    lock_set(fd, F_UNLCK);
    fclose(fp);

    /* 将源文件内容写入目标文件中去 */
    /* 打开目标文件 */
    sprintf(filename, "%s", desfilename);
    fp = fopen(filename, "wb+");
    if (NULL == fp){
        printf("copyfile: open desfile error\n");
        free(pmem);
        return -1;
    }

    /*给文件加写入锁*/
    fd = fileno(fp);
    lock_set(fd, F_WRLCK);

    /* 写入内容 */
    nwrite = fwrite(pmem, 1, textlen, fp);
    if (textlen != nwrite){
        printf("copyfile: write desfile error\n");
        free(pmem);
        lock_set(fd, F_UNLCK);
        fclose(fp);
        return -1;
    }

    /* 释放内存 */
    free(pmem);
    /* 关闭目标文件 */
    fflush(fp);

    /*给文件解锁*/
    lock_set(fd, F_UNLCK);
    fclose(fp);   

    return 0;
}
