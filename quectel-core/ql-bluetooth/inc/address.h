#ifndef QL_BT_ADDRESS
#define QL_BT_ADDRESS
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <hardware/hardware.h>
#include <hardware/bluetooth.h>

#ifdef __cplusplus
extern "C" {
#endif

bool IsAddressValid(char *address);
bool BdAddrFromString(char *p,bt_bdaddr_t *out);

#ifdef __cplusplus
}
#endif

#endif
