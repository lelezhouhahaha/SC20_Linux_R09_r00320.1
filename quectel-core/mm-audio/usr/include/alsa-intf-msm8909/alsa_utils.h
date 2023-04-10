#ifndef __ALSA_UTILS_H__
#define __ALSA_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

void *alsa_utils_malloc(size_t size, const char* func, int line);

#define utils_malloc(size)  alsa_utils_malloc((size), __func__, __LINE__)

#ifdef __cplusplus
}
#endif

#endif
