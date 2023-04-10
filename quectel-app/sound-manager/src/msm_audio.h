/*
 * Copyright:  (C) 2020 quectel All rights reserved.
 *  Filename:  msm_audio.h
 *   Version:  1.0.0
 *    Author:  Peeta <peeta.chen@quectel.com>
 * ChangeLog:  1, Release initial version on 2020.07.28
 */
#ifndef __MSM_AUDIO_H__
#define __MSM_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MSMAUDIO_DEBUG 1
#ifdef MSMAUDIO_DEBUG

#ifndef LOG_TAG
#define LOG_TAG "MSMAUDIO"
#endif
#include <cutils/log.h>

#define QLOGD_IF(fmt, ...)	ALOGD_IF(1, "[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define QLOGW_IF(fmt, ...)	ALOGW_IF(1, "[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define QLOGE_IF(fmt, ...)	ALOGE_IF(1, "[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define QLOGD(fmt, ...)		ALOGD("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define QLOGV(fmt, ...)		ALOGV("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define QLOGI(fmt, ...)		ALOGI("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define QLOGW(fmt, ...)		ALOGW("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define QLOGE(fmt, ...)		ALOGE("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)

#else

#define QLRILD_DEBUG 0
#define QLOGD_IF
#define QLOGW_IF
#define QLOGE_IF
#define QLOGD
#define QLOGV
#define QLOGI
#define QLOGW
#define QLOGE

#endif

int audio_get_volume(char *name, int *val);
int audio_set_volume(char *name, int val);

int audio_play_wav(int *handle, char *wavfile);
int audio_play_stop(int *handle);

int audio_stream_enable(int *handle, char *output);
int audio_stream_disable(int *handle, char *output);

int voice_stream_enable(int *handle, char *output, char *input);
int voice_stream_disable(int *handle, char *output, char *input);

int snd_card_init(int *handle);
int snd_card_exit(int *handle);

#ifdef __cplusplus
}
#endif

#endif
