/**
 *   Copyright:  (C) 2021 Quectel. All rights reserved.
 *    Filename:  qaudio.h
 * Description:  This is a header file of qaudio library
 *     Version:  1.0.0(20210429)
 *      Author:  fulinux <peeta.chen@quectel.com>
 *   ChangeLog:  0.1, Release initial version on 20210429
 *				 1.0, Release alpha version on 20210607
 */
#ifndef _QAUDIO_H_
#define _QAUDIO_H_

#include <stdlib.h>
#include <stdint.h>

#include <system/audio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	MODE_BLOCK = 0,
	MODE_NONBLOCK,
} mode_e;

typedef enum {
	FILE_WAV = 1,
	FILE_MP3,
	FILE_AAC,
	FILE_AAC_ADTS,
	FILE_FLAC,
	FILE_ALAC,
	FILE_VORBIS,
	FILE_WMA,
	FILE_AC3,
	FILE_AAC_LATM,
	FILE_EAC3,
	FILE_EAC3_JOC,
	FILE_DTS,
	FILE_MP2,
	FILE_APTX,
	FILE_TRUEHD,
	FILE_IEC61937,
	FILE_APE
} file_e;

typedef enum {
	AAC_LC = 1,
	AAC_HE_V1,
	AAC_HE_V2,
	AAC_LOAS
} aac_format_e;

typedef enum {
	WMA = 1,     
	WMA_PRO,     
	WMA_LOSSLESS
} wma_format_e;

typedef enum {
	CBK_EVENT_WRITE_READY, /* non blocking write completed */
	CBK_EVENT_DRAIN_READY, /* drain completed */
	CBK_EVENT_ERROR, /* stream hit some error */
	/* callback event from ADSP PP,
	 * corresponding payload will be
	 * sent as is to the client
	 */
	CBK_EVENT_ADSP = 0x100,
	CBK_EVENT_DIED = 0x200, /* audio server died */
} stream_event_t;

typedef void (*error_callback_t)(void *context);
typedef int (*stream_callback_t)(int event, void *param, void *cookie);

int QAUDIO_Init(int *handle);
int QAUDIO_Exit(int *handle);
int QAUDIO_GetVersion(char *version, size_t size);

int QAUDIO_SetOutputCallback(int *handle, stream_callback_t func, void *data);//unused
int QAUDIO_GetCallbackPrivateData(int *handle, void **data);//unused
int QAUDIO_RegisterDeathNotify(int *handle, error_callback_t func, void *context);

int QAUDIO_GetOutputFlags(int *handle, uint32_t *dev, uint32_t *flags);
int QAUDIO_SetOutputFlags(int *handle, uint32_t dev, uint32_t flags);
int QAUDIO_GetOutputConfig(int *handle, int *format, int *rate, int *channels);
int QAUDIO_SetOutputConfig(int *handle, int format, int rate, int channels);
int QAUDIO_GetOutputVolume(int *handle, uint8_t *val);
int QAUDIO_SetOutputVolume(int *handle, uint8_t val);
int QAUDIO_AddOutputVolume(int *handle, int val);
int QAUDIO_DecOutputVolume(int *handle, int val);

int QAUDIO_OpenOutputDevice(int *handle, uint32_t dev, uint32_t flags);
int QAUDIO_SwitchOutputDevice(int *handle, uint32_t dev, uint32_t flags);
int QAUDIO_CloseOutputDevice(int *handle);
int QAUDIO_OutputInStandby(int *handle);
int QAUDIO_GetOutputBuffSize(int *handle, size_t *bytes);
int QAUDIO_GetPlaybackBuffSize(int *handle, size_t *bytes);
int QAUDIO_SetPlaybackBuffSize(int *handle, size_t bytes);
int QAUDIO_WriteOutputStream(int *handle, void *data, size_t size);

/* API unsureness begin */
int QAUDIO_OpenInputDevice(int *handle, uint32_t dev, uint32_t flags);
int QAUDIO_CloseInputDevice(int *handle);
int QAUDIO_InputInStandby(int *handle);
int QAUDIO_GetInputBuffSize(int *handle, size_t *size);
int QAUDIO_ReadInputStream(int *handle, void *data, size_t size);
/* API unsureness end */

int QAUDIO_GetInputFlags(int *handle, uint32_t *dev, uint32_t *flags);
int QAUDIO_SetInputFlags(int *handle, uint32_t dev, uint32_t flags);
int QAUDIO_GetInputConfig(int *handle, int *format, int *rate, int *channels);
int QAUDIO_SetInputConfig(int *handle, int format, int rate, int channels);
int QAUDIO_GetRecordParam(int *handle, double *delay);
int QAUDIO_SetRecordParam(int *handle, double delay);
int QAUDIO_StartRecordWav(int *handle, char *file, double length);
int QAUDIO_StopRecordWav(int *handle);

int QAUDIO_OpenMusicFile(int *handle, char *file, file_e type);
int QAUDIO_CloseMusicFile(int *handle);
int QAUDIO_SeekMusicFile(int *handle, int offset, int whence);
int QAUDIO_SetAACFormat(int *handle, aac_format_e type);
int QAUDIO_SetWMAFormat(int *handle, wma_format_e type);
int QAUDIO_PlayMusic(int *handle, mode_e mode);
int QAUDIO_StopMusic(int *handle);
int QAUDIO_PausePlayback(int *handle);
int QAUDIO_ResumePlayback(int *handle);

int QAUDIO_PrepareVoiceCall(int *handle, uint32_t dev, uint32_t flags);
int QAUDIO_StartVoiceCall(int *handle);
int QAUDIO_StopVoiceCall(int *handle);
int QAUDIO_IsInVoiceCall(int *handle);
int QAUDIO_GetVoiceVolume(int *handle, uint8_t *val);
int QAUDIO_SetVoiceVolume(int *handle, uint8_t val);
int QAUDIO_AddVoiceVolume(int *handle, int val);
int QAUDIO_DecVoiceVolume(int *handle, int val);
int QAUDIO_GetVoiceTxMute(int *handle, int *enable);
int QAUDIO_SetVoiceTxMute(int *handle, int enable);
int QAUDIO_GetVoiceRxMute(int *handle, int *enable);
int QAUDIO_SetVoiceRxMute(int *handle, int enable);
int QAUDIO_GetHandsFree(int *handle, int *enable);
int QAUDIO_SetHandsFree(int *handle, int enable);

int QAUDIO_OpenWavFile(int *handle, char *file);
int QAUDIO_CloseWavFile(int *handle);
int QAUDIO_PlayWav(int *handle, uint32_t mode);
int QAUDIO_StopWav(int *handle);

int QAUDIO_StartLoopback(int *handle, uint32_t source, uint32_t sink);
int QAUDIO_StopLoopback(int *handle);
#ifdef __cplusplus
}
#endif

#endif
