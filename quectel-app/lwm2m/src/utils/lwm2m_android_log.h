#ifndef LWM2M_LOG_H
#define LWM2M_LOG_H

//#include <android/log.h>

//#define DEBUG 1

#ifndef LOG_TAG
#define LOG_TAG    "Mango-lwm2m"
#endif
#include "internals.h"

#define LOGD(...)
#define LOGI(...)
#define LOGW(...)
#define LOGE(...)
#define LOGF(...)

#if 0
#define LOGD(...)  fprintf(stdout,"\r",__VA_ARGS__);
#define LOGI(...)  fprintf(stdout,"\r",__VA_ARGS__);
#define LOGW(...)  fprintf(stdout,"\r",__VA_ARGS__);
#define LOGE(...)  fprintf(stdout,"\r", __VA_ARGS__);
#define LOGF(...)  fprintf(stdout,"\r", __VA_ARGS__);
#endif

#endif //MODEMTOOLTEST_QL_LOG_H

