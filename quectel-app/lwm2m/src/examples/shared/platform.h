#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include "liblwm2m.h"

#define MAX_SIZE 125

int write_data(void* data, int length);
int read_data(void* data, int length);
int delete_data();
int dump_connect_info();
int parse_connect_info(bool* isColdStart, char *server, char* serverPort, char* name, char* pskId, char* psk, uint16_t* pskLen);

//add by joe start
int write_data_for_vzw(int instanceId,void* data, int length);
int read_data_for_vzw(int instanceId,void* data, int length);
int delete_data_for_vzw(int instanceId);
int dump_connect_info_for_vzw(int instanceId);
int parse_connect_info_for_vzw(int instanceId,bool* isColdStart, char *server, char* serverPort, char* name, char* pskId, char* psk, uint16_t* pskLen);
bool parse_is_bootstrapd_for_vzw();
//add by joe end
bool within_life_time();
#endif