/**
 *   Copyright:  (C) 2021 Quectel. All rights reserved.
 *    Filename:  qaudio-api-test.c
 * Description:  This file is used to test the QAUDIO API
 *     Version:  1.0.0(20210429)
 *      Author:  Peeta Chen/fulinux <peeta.chen@quectel.com>
 *   ChangeLog:  1, Release initial version on 20210429
 */

#include <stdio.h>
#include <stdlib.h>
#include <qaudio.h>
#include <math.h>

#include "wav.h"

static pthread_mutex_t s_stateMutex;

typedef struct {
	int cmdIdx;
	char *desc;
} st_api_test_case;

typedef struct {
	char *group_name;
	st_api_test_case *test_cases;
} func_api_test_t;

int stop_play = 0;
int pause_play = 0;

st_api_test_case api_testlist[] = {
	{0, "Help, Show all the API"},

	{-2, "========================================\n"
		 "General Functions\n"
		 "========================================"},
	{1, "QAUDIO_Init"},
	{2, "QAUDIO_Exit"},

	{-2, "========================================\n"
		 "Output Stream Functions\n"
		 "========================================"},
	{10, "QAUDIO_GetOutputFlags"},
	{11, "QAUDIO_SetOutputFlags"},
	{12, "QAUDIO_GetOutputConfig"},
	{13, "QAUDIO_SetOutputConfig"},
	{14, "QAUDIO_GetOutputVolume"},
	{15, "QAUDIO_SetOutputVolume"},
	{16, "QAUDIO_OpenOutputDevice"},
	{17, "QAUDIO_CloseOutputDevice"},
	{18, "QAUDIO_AddOutputVolume"},
	{19, "QAUDIO_DecOutputVolume"},
	{28, "QAUDIO_GetPlaybackBuffSize"},
	{29, "QAUDIO_SetPlaybackBuffSize"},
	{70, "QAUDIO_GetOutputBuffSize"},

	{-2, "========================================\n"
		 "Play music file Functions\n"
		 "========================================"},
	{20, "QAUDIO_OpenMusicFile"},
	{21, "QAUDIO_CloseMusicFile"},
	{22, "QAUDIO_PlayMusic"},
	{23, "QAUDIO_StopMusic"},
	{24, "QAUDIO_PausePlayback"},
	{25, "QAUDIO_ResumePlayback"},
	{26, "QAUDIO_SetAACFormat"},
	{27, "QAUDIO_SetWMAFormat"},

	{-2, "========================================\n"
		 "Input stream Functions\n"
		 "========================================"},
	{30, "QAUDIO_GetInputFlags"},
	{31, "QAUDIO_SetInputFlags"},
	{32, "QAUDIO_GetInputConfig"},
	{33, "QAUDIO_SetInputConfig"},
	{34, "QAUDIO_GetRecordParam"},
	{35, "QAUDIO_SetRecordParam"},
	{36, "QAUDIO_StartRecordWav"},
	{37, "QAUDIO_StopRecordWav"},
	{38, "QAUDIO_StartLoopback"},
	{39, "QAUDIO_StopLoopback"},

	{-2, "========================================\n"
		 "Voice in call Functions\n"
		 "You should stop sound-manager daemon\n"
		 "before testing the following APIs\n"
		 "========================================"},
	{40, "QAUDIO_PrepareVoiceCall"},
	{41, "QAUDIO_StartVoiceCall"},
	{42, "QAUDIO_StopVoiceCall"},
	{43, "QAUDIO_GetVoiceVolume"},
	{44, "QAUDIO_SetVoiceVolume"},
	{45, "QAUDIO_GetVoiceTxMute"},
	{46, "QAUDIO_SetVoiceTxMute"},
	{47, "QAUDIO_GetVoiceRxMute"},
	{48, "QAUDIO_SetVoiceRxMute"},
	{49, "QAUDIO_GetHandsFree"},
	{50, "QAUDIO_SetHandsFree"},
	{51, "QAUDIO_AddVoiceVolume"},
	{52, "QAUDIO_DecVoiceVolume"},

	{-2, "========================================\n"
		 "examples of combinatorial testing from the above APIs\n"
		 "========================================"},
	{90, "Playing WAV file"},
	{91, "Playing WAV file by another way"},
	{92, "Playing MP3 file"},
	{93, "Playing loop (busy tone wave file)"},
	{94, "Recording of WAV from EVB mic(the noise is big:)"},
	{95, "Recording of WAV from wired headset"},
	{96, "Playing Two WAV file with stream"},
	{97, "Playing Two WAV file with HPH-stereo SPK-mono"},
	{98, "Change the output device between speaker and headset when playing."},
	{99, "Change the output device between speaker and headset when playing with bytes stream"},

	{-1, "Exit!"}
};

func_api_test_t qaudio_api_test = {"Quectel Audio APIs", api_testlist};

static void usage(func_api_test_t *pt_test)
{
	int i;

	printf("Copyright (C) 2021 Quectel, Smart Linux\n");
	printf("Group Name:%s, Supported test cases:\n", pt_test->group_name);

	for (i = 0;; i++) {
		if (pt_test->test_cases[i].cmdIdx == -2) {
			printf ("%s\n", pt_test->test_cases[i].desc);
			continue;
		}

		printf("%d:\t%s\n", pt_test->test_cases[i].cmdIdx, pt_test->test_cases[i].desc);
		if (pt_test->test_cases[i].cmdIdx == -1)
			break;
	}
}

static int getInputIntVal(void)
{
	int n = 0;
	int ret = 0;
	char cmdStr[10] = {0};
	char *p = NULL;
	char ch;

	memset(cmdStr, 0, sizeof(cmdStr));
	if (fgets(cmdStr, sizeof(cmdStr), stdin) == NULL)
		return -1;

	if (cmdStr[0] == '\n')
		return -2;

	p = cmdStr;
	while (*p) {
		if (sscanf(p, "%d%n", &ret, &n) == 1) {
			p += n;
		} else
			++p;
	}

	return ret;
}

static int getInputHexVal(void)
{
	int n = 0;
	int ret = 0;
	char cmdStr[10] = {0};
	char *p = NULL;
	char ch;

	memset(cmdStr, 0, sizeof(cmdStr));
	if (fgets(cmdStr, sizeof(cmdStr), stdin) == NULL)
		return -1;

	if (cmdStr[0] == '\n')
		return -2;

	p = cmdStr;
	while (*p) {
		if (sscanf(p, "%x%n", &ret, &n) == 1) {
			p += n;
		} else
			++p;
	}

	return ret;
}

static int getInputString(char *str, size_t size)
{
	int i = 0;
	int ret = 0;

	if (str == NULL || size <= 0) {
		return -1;
	}

	memset(str, 0, size);
	if (fgets(str, size, stdin) == NULL)
		return 0;

	for (i = 0; i < size; i++) {
		if (str[i] == '\n') {
			str[i] = '\0';
			break;
		}
	}

	ret = strlen(str);

	return ret;
}

static void generate_sine_wave(float *data, int rate, int len)
{
	int i = 0;
	for (i = 0; i < len; ++i) {
		data[i] = 0.5f * cosf(2 * 3.14159265358979323f * 440.0f * i / rate);
	}
}

static int process_wav_file(WavFile **pwav, const char *filename, int *format, size_t *sample_size, int *channels, int *rate)
{
	size_t size = 0;

	*pwav = wav_open(filename, WAV_OPEN_READ);
	if (pwav == NULL) {
		printf("%s: wav_open error\n", filename);
		return -1;
	} else
		printf("%s: wav_open: success\n", filename);

	*format = wav_get_format(*pwav);
	*sample_size = wav_get_sample_size(*pwav);
	if (*format == WAV_FORMAT_PCM && *sample_size == 1)//16bit pcm
		*format = AUDIO_FORMAT_PCM_8_BIT;
	else if (*format == WAV_FORMAT_PCM && *sample_size == 4)//16bit pcm
		*format = AUDIO_FORMAT_PCM_32_BIT;
	else if (*format == WAV_FORMAT_IEEE_FLOAT)
		*format = AUDIO_FORMAT_PCM_FLOAT;
	else
		*format = AUDIO_FORMAT_PCM_16_BIT;

	printf ("wave file format code:0x%X\n", *format);
	printf ("wave file sample size:%d\n", *sample_size);

	*channels = wav_get_num_channels(*pwav);
	printf ("wave file number of channels:%d\n", *channels);
	if (*channels < 1)
		*channels = 1;
	else if (*channels > 2)
		*channels = 2;

	*rate = wav_get_sample_rate(*pwav);
	printf ("wave file sample rate:%d\n", *rate);

	size = wav_get_length(*pwav);
	printf ("wave file length:%d\n", size);

	return 0;
}

static void *play_wav_file_with_stream(void)
{
	int hd =0;
	uint32_t dev = 0;
	uint32_t flags = 0;
	WavFile *pwav = NULL;
	int bytes_remaining = 0;
	int size = 0;
	int offset = 0;
	size_t buffsize = 0;
	size_t bytes_wanted = 0;
	int format = AUDIO_FORMAT_PCM_16_BIT;
	int rate = 44100;
	size_t sample_size = 2;
	int channels = 2;
	void *buff = NULL;
	int ret = 0;
	int err = 0;
	uint8_t level = 0;

	printf("thread start\n");

	pthread_mutex_init(&s_stateMutex, NULL);
	pthread_mutex_lock(&s_stateMutex);
	if (hd == 0) {
	ret = QAUDIO_Init(&hd);
	if (ret == 0)
		printf("QAUDIO_Init success\n");
	else
		printf("QAUDIO_Init error:%d\n", ret);
	} else
		printf("QAUDIO_Init has executed already\n");

	ret = process_wav_file(&pwav, "/etc/mmi/qualsound.wav", &format, &sample_size, &channels, &rate);
	if(ret < 0) {
		printf("process_wav_file error:%d\n", ret);
		err = 1;
		goto next;
	} else
		printf("process_wav_file: success\n");

	ret = QAUDIO_SetOutputConfig(&hd, format, rate, channels);
	if (ret < 0) {
		printf("QAUDIO_SetOutputConfig error:%d\n", ret);
		err = 1;
		goto next;
	} else
		printf("QAUDIO_SetOutputConfig: success\n");

	dev = AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
	flags = AUDIO_OUTPUT_FLAG_NON_BLOCKING;

	ret = QAUDIO_OpenOutputDevice(&hd, dev, flags);
	if (ret < 0) {
		printf("QAUDIO_OpenOutputDevice error:%d\n", ret);
		err = 1;
		goto next;
	} else
		printf("QAUDIO_OpenOutputDevice: success\n");

	level = 200; //0 < level <= 255
	ret = QAUDIO_SetOutputVolume(&hd, level);
	if (ret < 0) {
		printf("QAUDIO_SetOutputVolume error:%d\n", ret);
		err = 1;
		goto next;
	} else
		printf("QAUDIO_SetOutputVolume: second success\n");

	ret = QAUDIO_GetOutputBuffSize(&hd, &buffsize);
	if (ret < 0) {
		printf("QAUDIO_GetOutputBuffSize error:%d\n", ret);
		err = 1;
		goto next;
	} else
		printf("QAUDIO_GetOutputBuffSize buffer size[%d]\n", buffsize);

	buff = (void *)malloc(buffsize);
	if (buff == NULL) {
		printf("malloc failed\n");
		err = 1;
		goto next;
	}

	pthread_mutex_unlock(&s_stateMutex);
	do {
			if (bytes_remaining == 0) {
				bytes_wanted = buffsize/channels/sample_size;
				size = wav_read(pwav, buff, bytes_wanted);
				if (size == 0) {
					break;
				}

				offset = 0;
				bytes_remaining = size * channels * sample_size;
			}

			ret = QAUDIO_WriteOutputStream(&hd, (char *)(buff + offset), bytes_remaining);
			//printf("bytes_remaining[%d] ret[%d]\n", bytes_remaining, ret);
			if (ret < 0) {
				printf("QAUDIO_WriteOutputStream error:%d\n", ret);
				break;
			} else {
				offset = ret;
				bytes_remaining -= ret;
			}

		} while(size > 0);
next:
		if(err)
			pthread_mutex_unlock(&s_stateMutex);

		if (buff) {
			free(buff);
			buff = NULL;
		}

		if (pwav) {
			wav_close(pwav);
			pwav = NULL;
		}

		ret = QAUDIO_CloseOutputDevice(&hd);
		if (ret < 0)
			printf("QAUDIO_CloseOutputDevice error:%d\n", ret);
		else
			printf("QAUDIO_CloseOutputDevice: success\n");

		if (QAUDIO_Exit(&hd) == 0)
			printf("QAUDIO_Exit success\n");
		else
			printf("QAUDIO_Exit failure\n");

	printf("thread over\n");
}

int qaudio_async_callback(int event, void *param, void *cookie)
{
	int i = 0;
	int ret = 0;
	void *fool = NULL;//for example
	int *handle = NULL;
	uint32_t *playload = NULL;

	printf ("%s: Enter\n", __func__);

	handle = (int *)cookie;
	if (handle == NULL) {
		printf("Invalid callback cookie - handle\n");
	}

	printf ("%s: event: %d\n", __func__, event);
	ret = QAUDIO_GetCallbackPrivateData(handle, fool);
	if (ret == 0)
		printf("QAUDIO_GetCallbackPrivateData fool = %d\n", *(int *)fool);

	printf ("%s: event: %d\n", __func__, event);
	switch(event) {
		case CBK_EVENT_WRITE_READY:
			printf("stream received event - CBK_EVENT_WRITE_READY\n");
			break;
		case CBK_EVENT_DRAIN_READY:
			printf("stream received event - CBK_EVENT_DRAIN_READY\n");
			break;
		case CBK_EVENT_ADSP:
			printf("stream received event - CBK_EVENT_ADSP\n");
			playload = (uint32_t *)param;
			if (playload != NULL) {
				printf("event type:%d\n", playload[0]);
				printf("event length:%d\n", playload[1]);
				for (i = 2; (i * sizeof(uint32_t)) < playload[1]; i++)
					printf("event data[]:%d\n", playload[i]);
			}
			break;
		case CBK_EVENT_ERROR:
			printf("stream received event - CBK_EVENT_ERROR\n");
			break;
		case CBK_EVENT_DIED:
			printf("stream received event - CBK_EVENT_DIED\n");
			break;
		default:
			printf("stream received event - UNKNOWN\n");
			break;
	}

	printf ("%s: Exit\n", __func__);

	return 0;
}

static void *switch_device(void *hd)
{
	uint32_t dev, flags;
	int tmp = 0;
	int ret = 0;

	pthread_mutex_lock(&s_stateMutex);

	flags = AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD | AUDIO_OUTPUT_FLAG_DIRECT;

	while(1) {

		sleep(20);

		if (tmp%2 == 0) {
			dev = AUDIO_DEVICE_OUT_SPEAKER;
			printf("Playing music on speaker for 20 seconds...\n");
		} else {
			dev = AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
			printf("Playing music on headphone for 20 seconds...\n");
		}

		tmp++;
		pause_play = 1;

		ret = QAUDIO_SwitchOutputDevice((int *)hd, dev, flags);
		if (ret < 0) {
			printf("QAUDIO_SwitchOutputDevice error:%d\n", ret);
			pthread_mutex_unlock(&s_stateMutex);
			break;
		} else
			printf("QAUDIO_SwitchOutputDevice: success\n");

		pause_play = 0;

		if (tmp == 4) {
			sleep(20);
			pthread_mutex_unlock(&s_stateMutex);
			break;
		}
	}

	stop_play = 1;
}


int main (int argc, char **argv)
{
	int i = 0;
	int hd = 0;
	int ret = 0;
	int cmdIdx = 0;
	int enable = 0;
	uint8_t level = 0;
	uint32_t dev, flags;
	char buf[512] = {0};
	int format = AUDIO_FORMAT_PCM_16_BIT;
	int rate = 44100;
	int channels = 2;


	for (i = 0; i < 5; i++) {
		ret = QAUDIO_Init(&hd);
		if (ret == 0) {
			printf("QAUDIO_Init success\n");
			break;
		}

		sleep(1);
	}

	ret = QAUDIO_GetVersion(buf, sizeof(buf));
	if (ret == 0)
		printf("QAUDIO library current version:%s\n", buf);

	if (hd == 0 || i >= 5) {
		printf("QAUDIO_Init failed:%d", ret);
		return -1;
	}

	usage(&qaudio_api_test);

	for (;;) {
		printf("\n");
		printf("Please input cmd index (-1: exit, 0: help):");
		cmdIdx = getInputIntVal();

		if (cmdIdx == -1) {
			if (hd != 0) {
				ret = QAUDIO_Exit(&hd);
				if (ret == 0)
					printf("QAUDIO_Exit success\n");
				else
					printf("QAUDIO_Exit error:%d\n", ret);
			}
			break;
		}

		if (cmdIdx == -2)
			continue;

		switch (cmdIdx) {
			case 0:
				usage(&qaudio_api_test);
				break;
			case 1:
				if (hd == 0) {
					ret = QAUDIO_Init(&hd);
					if (ret == 0)
						printf("QAUDIO_Init success\n");
					else
						printf("QAUDIO_Init error:%d\n", ret);
				} else
					printf("QAUDIO_Init has executed already\n");
				break;
			case 2:
				if (QAUDIO_Exit(&hd) == 0)
					printf("QAUDIO_Exit success\n");
				else
					printf("QAUDIO_Exit failure\n");
				break;
			case 10:
				ret = QAUDIO_GetOutputFlags(&hd, &dev, &flags);
				if (ret != 0)
					printf("QAUDIO_GetOutputFlags error:%d\n", ret);
				else
					printf("QAUDIO_GetOutputFlags dev[0x%X] flags[0x%X]\n", dev, flags);
				break;
			case 11:
				printf("input device(e.g 1:EARPIECE 2:SPEAKER 4:WIRED_HEADSET 8:WIRED_HEADPHONE -1:back):");
				ret = getInputIntVal();
				if (ret < 0)
					break;

				dev = ret & 0xF;//see audio_devices_t in audio.h

				/* see audio_output_flags_t in audio.h */
				printf("input hex value of flags(-1:back 0:NONE 1:DIRECT 2:PRIMARY 4:FAST 8:DEEP_BUFFER\n");
				printf("10: COMPRESS_OFFLOAD 20:NON_BLOCKING 40:HW_AV_SYNC 80:FLAG_TTS or combination:");
				ret = getInputHexVal();
				if (ret < 0)
					break;

				flags = (uint32_t)ret;

				/* dev: default is AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_WIRED_HEADPHONE/
				 * flags: default is AUDIO_OUTPUT_FLAG_PRIMARY
				 */
				ret = QAUDIO_SetOutputFlags(&hd, dev, flags);
				if (ret < 0)
					printf("QAUDIO_SetOutputFlags error:%d\n", ret);
				else
					printf("QAUDIO_SetOutputFlags: success\n");
				break;
			case 12:
				ret = QAUDIO_GetOutputConfig(&hd, &format, &rate, &channels);
				if (ret < 0)
					printf("QAUDIO_GetOutputConfig error:%d\n", ret);
				else
					printf("QAUDIO_GetOutputConfig: format[0x%X] rate[%d] channels[%d]\n",
							format, rate, channels);
				break;
			case 13:
			{
				printf("input hex value of format(e.g 0: PCM, 1:PCM_16Bit, 2:PCM_8Bit,\n");
				printf("1000000:MP3, 2000000:AMR_NB, 3000000:AMR_WB, 4000000:AAC, ...):");
				format = getInputHexVal();
				if (format < 0)
					break;

				printf("input value of sample rate(e.g 8000, 44100, 48000, -1:back):");
				rate = getInputIntVal();
				if (rate < 0)
					break;
				else if (rate > 48000)
					rate = 48000;

				printf("input value of channels(e.g 1 or 2, -1:back):");
				channels = getInputIntVal();
				if (channels < 0)
					break;
				else if (channels != 1 && channels != 2)
					channels = 2;

				/* format: default is AUDIO_FORMAT_PCM_16_BIT
				 * rate: default is 48000
				 * channels: default is AUDIO_CHANNEL_OUT_STEREO
				 * bit: default is 16
				 */
				ret = QAUDIO_SetOutputConfig(&hd, format, rate, channels);
				if (ret < 0)
					printf("QAUDIO_SetOutputConfig error:%d\n", ret);
				else
					printf("QAUDIO_SetOutputConfig: success\n");
				break;
			}
			case 14:
			{
				ret = QAUDIO_GetOutputVolume(&hd, &level);
				if (ret < 0)
					printf("QAUDIO_GetOutputVolume error:%d\n", ret);

				printf("Output volume level:%d\n", level);
				break;
			}
			case 15:
			{
				printf("input value of volume level(0 ~ 255, -1:back):");
				ret = getInputIntVal();
				if (ret < 0)
					break;
				else if (ret > 255)
					ret = 255;

				level = ret;

				ret = QAUDIO_SetOutputVolume(&hd, level);
				if (ret < 0)
					printf("QAUDIO_SetOutputVolume error:%d\n", ret);
				else
					printf("Setting output volume level success\n");
				break;
			}
			case 16:
			{
				printf("input device(e.g 1:EARPIECE 2:SPEAKER 4:WIRED_HEADSET 8:WIRED_HEADPHONE -1:back):");
				ret = getInputIntVal();
				if (ret < 0)
					break;

				dev = ret & 0xF;//see audio_devices_t in audio.h

				/* see audio_output_flags_t in audio.h */
				printf("input hex value of flags(-1:back 0:NONE 1:DIRECT 2:PRIMARY 4:FAST 8:DEEP_BUFFER\n");
				printf("10: COMPRESS_OFFLOAD 20:NON_BLOCKING 40:HW_AV_SYNC 80:FLAG_TTS or combination:");
				ret = getInputHexVal();
				if (ret < 0)
					break;

				flags = (uint32_t)ret;

				/* dev: default is AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_WIRED_HEADPHONE/
				 * flags: default is AUDIO_OUTPUT_FLAG_PRIMARY
				 */
				ret = QAUDIO_OpenOutputDevice(&hd, dev, flags);
				if (ret < 0)
					printf("QAUDIO_OpenOutputDevice error:%d\n", ret);
				else
					printf("QAUDIO_OpenOutputDevice: success\n");
				break;
			}
			case 17:
			{
				ret = QAUDIO_CloseOutputDevice(&hd);
				if (ret < 0)
					printf("QAUDIO_CloseOutputDevice error:%d\n", ret);
				else
					printf("QAUDIO_CloseOutputDevice: success\n");
				break;
			}
			case 18:
			{
				ret = QAUDIO_AddOutputVolume(&hd, 5);//5 - for example
				if (ret < 0)
					printf("QAUDIO_AddOutputVolume error:%d\n", ret);
				else
					printf("QAUDIO_AddOutputVolume success\n");
				break;
			}
			case 19:
			{
				ret = QAUDIO_DecOutputVolume(&hd, 5);//5 - for example
				if (ret < 0)
					printf("QAUDIO_DecOutputVolume error:%d\n", ret);
				else
					printf("QAUDIO_DecOutputVolume success\n");
				break;
			}
			case 28:
			{
				size_t size = 0;
				ret = QAUDIO_GetPlaybackBuffSize(&hd, &size);
				if (ret < 0)
					printf("QAUDIO_GetPlaybackBuffSize error:%d\n", ret);
				else
					printf("Playback buffer size of wanted:%d\n", size);
				break;
			}
			case 29:
			{
				size_t size = 0;
				printf("input value of buffer size(e.g. 1024 -1: back):");
				ret = getInputIntVal();
				if (ret < 0)
					break;
				size = (size_t)ret;

				ret = QAUDIO_SetPlaybackBuffSize(&hd, size);
				if (ret < 0)
					printf("QAUDIO_SetPlaybackBuffSize error:%d\n", ret);
				else
					printf("QAUDIO_SetPlaybackBuffSize success\n");
				break;
			}
			case 70:
			{
				size_t size = 0;
				ret = QAUDIO_GetOutputBuffSize(&hd, &size);
				if (ret < 0)
					printf("QAUDIO_GetOutputBuffSize error:%d\n", ret);
				else
					printf("Output buffer size:%d\n", size);
				break;
			}
			case 200://unused
			{
				int fool = 123;
				ret = QAUDIO_SetOutputCallback(&hd, qaudio_async_callback, (void *)&fool);
				if (ret < 0)
					printf("QAUDIO_SetOutputCallback error:%d\n", ret);
				else
					printf("QAUDIO_SetOutputCallback: success\n");
				break;
			}
			case 20:
			{
				file_e type = FILE_WAV;

				printf("input music file(-1: back, default is /etc/mmi/qualsound.wav):");
				ret = getInputString(buf, sizeof(buf));
				if (strncmp(buf, "-1", 2) == 0)
					break;
				else if (ret < 3)
					snprintf(buf, sizeof(buf), "%s", "/etc/mmi/qualsound.wav");

				printf("input value of file type(e.g. 1:WAV(default) 2:MP3 3:AAC -1: back):");
				ret = getInputIntVal();
				if (ret < 0)
					break;
				else if (ret > FILE_APE)
					ret = 1;

				type = (file_e)ret;

				ret = QAUDIO_OpenMusicFile(&hd, buf, type);
				if (ret < 0)
					printf("QAUDIO_OpenMusicFile error:%d\n", ret);
				else
					printf("QAUDIO_OpenMusicFile: success\n");

				break;
			}
			case 21:
				ret = QAUDIO_CloseMusicFile(&hd);
				if (ret < 0)
					printf("QAUDIO_CloseMusicFile error:%d\n", ret);
				else
					printf("QAUDIO_CloseMusicFile: success\n");
				break;
			case 22:
			{
				int mode = 0;
				printf("input value of play mode(e.g. 0:block(default) 1:non-block -1: back):");
				mode = getInputIntVal();
				if (mode < 0)
					break;
				if (mode != 1)
					mode = 0;

				ret = QAUDIO_PlayMusic(&hd, mode);
				if (ret < 0)
					printf("QAUDIO_PlayMusic error:%d\n", ret);
				else
					printf("QAUDIO_PlayMusic: success\n");

				break;
			}
			case 23:
				ret = QAUDIO_StopMusic(&hd);
				if (ret < 0)
					printf("QAUDIO_StopMusic error:%d\n", ret);
				else
					printf("QAUDIO_StopMusic: success\n");

				break;
			case 24:
				ret = QAUDIO_PausePlayback(&hd);
				if (ret < 0)
					printf("QAUDIO_PausePlayback error:%d\n", ret);
				else
					printf("QAUDIO_PausePlayback: success\n");

				break;
			case 25:
				ret = QAUDIO_ResumePlayback(&hd);
				if (ret < 0)
					printf("QAUDIO_ResumePlayback error:%d\n", ret);
				else
					printf("QAUDIO_ResumePlayback: success\n");

				break;
			case 26:
			{
				aac_format_e aac;

				printf("input AAC format(e.g. 1:AAC_LC 2:AAC_HE_V1 3:AAC_HE_V2 4:AAC_LOAS -1:back):");
				ret = getInputIntVal();
				if (ret < 0)
					break;
				else if (ret > 4)
					ret = 1;

				aac = (aac_format_e)ret;

				ret = QAUDIO_SetAACFormat(&hd, aac);
				if (ret < 0)
					printf("QAUDIO_SetAACFormat error:%d\n", ret);
				else
					printf("QAUDIO_SetAACFormat: success\n");

				break;
			}
			case 27:
			{
				wma_format_e wma;

				printf("input WMA format(e.g. 1:WMA 2:WMA_PRO 3:WMA_LOSSLESS -1:back):");
				ret = getInputIntVal();
				if (ret < 0)
					break;
				else if (ret > 3)
					ret = 1;

				wma = (wma_format_e)ret;

				ret = QAUDIO_SetWMAFormat(&hd, wma);
				if (ret < 0)
					printf("QAUDIO_SetWMAFormat error:%d\n", ret);
				else
					printf("QAUDIO_SetWMAFormat: success\n");

				break;
			}
			case 30:
			{
				ret = QAUDIO_GetInputFlags(&hd, &dev, &flags);
				if (ret < 0)
					printf("QAUDIO_GetInputFlags error:%d\n", ret);
				else
					printf("Input stream dev[0x%X] flags[0x%X]\n", dev, flags);

				break;
			}
			case 31:
			{
				printf("input hex value of device(e.g 1:COMMUNICATION 2:AMBIENT\n"\
						"4:BUILTIN_MIC(default) 10:WIRED_HEADSET -1:back):");
				ret = getInputHexVal();
				if (ret < 0)
					break;
				else if (ret == 0)
					ret = 0x4; //0x80000004:AUDIO_DEVICE_IN_BUILTIN_MIC

				dev = (uint32_t)ret;//see audio_devices_t in audio.h
				dev |= AUDIO_DEVICE_BIT_IN;

				/* see audio_output_flags_t in audio.h */
				printf("input hex value of flags(-1:back 0:NONE 1:FAST 2:HW_HOTWORD\n");
				printf("4: RAW 8:SYNC 10:MMAP_NOIRQ 40000000:COMPRESS 80000000:TIMESTAMP:");
				ret = getInputHexVal();
				if (ret == -1)
					break;

				flags = (uint32_t)ret;

				ret = QAUDIO_SetInputFlags(&hd, dev, flags);
				if (ret < 0)
					printf("QAUDIO_SetInputFlags error:%d\n", ret);
				else
					printf("QAUDIO_SetInputFlags: success\n");

				break;
			}
			case 32:
			{
				ret = QAUDIO_GetInputConfig(&hd, &format, &rate, &channels);
				if (ret < 0)
					printf("QAUDIO_GetInputConfig error:%d\n", ret);
				else
					printf("QAUDIO_GetInputConfig: format[0x%X] rate[%d] channels[%d]\n",
							format, rate, channels);
				break;
			}
			case 33:
			{
				printf("input hex value of format(e.g 0: PCM, 1:PCM_16Bit, 2:PCM_8Bit,\n");
				printf("1000000:MP3, 2000000:AMR_NB, 3000000:AMR_WB, 4000000:AAC, ...):");
				format = getInputHexVal();
				if (format < 0)
					break;

				printf("input value of sample rate(e.g 8000, 44100, 48000, -1:back):");
				rate = getInputIntVal();
				if (rate < 0)
					break;
				else if (rate > 48000)
					rate = 48000;

				printf("input value of channels(e.g 1 or 2, -1:back):");
				channels = getInputIntVal();
				if (channels < 0)
					break;
				else if (channels != 1 && channels != 2)
					channels = 1;

				/* format: default is AUDIO_FORMAT_PCM_16_BIT
				 * rate: default is 48000
				 * channels: default is AUDIO_CHANNEL_OUT_STEREO
				 * bit: default is 16
				 */
				ret = QAUDIO_SetInputConfig(&hd, format, rate, channels);
				if (ret < 0)
					printf("QAUDIO_SetInputConfig error:%d\n", ret);
				else
					printf("QAUDIO_SetInputConfig: success\n");
				break;
			}
			case 34:
			{
				double delay = 0;
				ret = QAUDIO_GetRecordParam(&hd, &delay);
				if (ret < 0)
					printf("QAUDIO_GetRecordParam error:%d\n", ret);
				else
					printf("QAUDIO_GetRecordParam delay:%f\n", delay);
				break;
			}
			case 35:
			{
				double delay = 0;

				printf("input value to delay recording(e.g 1 2 ..., -1:back):");
				ret = getInputIntVal();
				if (ret < 0)
					break;

				delay = (double)ret;//second

				ret = QAUDIO_SetRecordParam(&hd, delay);
				if (ret < 0)
					printf("QAUDIO_SetRecordParam error:%d\n", ret);
				else
					printf("QAUDIO_SetRecordParam: success\n");

				break;
			}
			case 36:
			{
				snprintf(buf, sizeof(buf), "%s", "/data/record.wav");

				printf("start recording (%s)\n", buf);
				ret = QAUDIO_StartRecordWav(&hd, buf, 5.0);//5s
				if (ret < 0)
					printf("QAUDIO_StartRecordWav error:%d\n", ret);
				else
					printf("QAUDIO_StartRecordWav: success\n");

				break;
			}
			case 37:
			{
				ret = QAUDIO_StopRecordWav(&hd);
				if (ret < 0)
					printf("QAUDIO_StopRecordWav error:%d\n", ret);
				else
					printf("Stop recording: success\n");

				break;
			}
			case 38:
			{
				uint32_t sink = 0;
				uint32_t source = 0;

				printf("input hex value of input device(e.g 1:COMMUNICATION 2:AMBIENT\n"\
						"4:BUILTIN_MIC(default) 10:WIRED_HEADSET -1:back):");
				ret = getInputHexVal();
				if (ret < 0)
					break;
				else if (ret == 0)
					ret = 0x4; //0x80000004:AUDIO_DEVICE_IN_BUILTIN_MIC

				source = (uint32_t)ret;//see audio_devices_t in audio.h
				source |= AUDIO_DEVICE_BIT_IN;

				printf("input hex value of output device(e.g 1:EARPIECE 2:SPEAKER\n"
						"4:WIRED_HEADSET 8:WIRED_HEADPHONE -1:back):");
				ret = getInputHexVal();
				if (ret < 0)
					break;
				sink = ret;

				ret = QAUDIO_StartLoopback(&hd, source, sink);
				if (ret < 0)
					printf("QAUDIO_StartLoopback error:%d\n", ret);
				else
					printf("QAUDIO StartLoopback: success\n");

				break;
			}
			case 39:
			{
				ret = QAUDIO_StopLoopback(&hd);
				if (ret < 0)
					printf("QAUDIO_StopLoopback error:%d\n", ret);
				else
					printf("QAUDIO StopLoopback: success\n");

				break;
			}
			case 40:
			{
				printf("input device(e.g 1:EARPIECE 2:SPEAKER 4:WIRED_HEADSET 8:WIRED_HEADPHONE -1:back):");
				ret = getInputIntVal();
				if (ret == -1)
					break;

				dev = ret & 0xF;//see audio_devices_t in audio.h

				/* see audio_output_flags_t in audio.h */
				printf("input hex value of flags(-1:back e.g. 0:NONE 1:DIRECT 2:PRIMARY(default):");
				ret = getInputHexVal();
				if (ret == -1)
					break;
				else if (ret > 0x10)
					ret = AUDIO_OUTPUT_FLAG_PRIMARY;

				flags = (uint32_t)ret;

				/* dev: default is AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_WIRED_HEADPHONE/
				 * flags: default is AUDIO_OUTPUT_FLAG_PRIMARY
				 */
				ret = QAUDIO_PrepareVoiceCall(&hd, dev, flags);
				if (ret < 0)
					printf("QAUDIO_PrepareVoiceCall error:%d\n", ret);
				else
					printf("QAUDIO_PrepareVoiceCall: success\n");

				break;
			}
			case 41:
			{
				ret = QAUDIO_StartVoiceCall(&hd);
				if (ret < 0)
					printf("QAUDIO_StartVoiceCall error:%d\n", ret);
				else
					printf("QAUDIO_StartVoiceCall: success\n");

				break;
			}
			case 42:
			{
				ret = QAUDIO_StopVoiceCall(&hd);
				if (ret < 0)
					printf("QAUDIO_StopVoiceCall error:%d\n", ret);
				else
					printf("QAUDIO_StopVoiceCall: success\n");

				break;
			}
			case 43:
			{
				ret = QAUDIO_GetVoiceVolume(&hd, &level);
				if (ret < 0)
					printf("QAUDIO_GetVoiceVolume error:%d\n", ret);
				else
					printf("Voice volume:%d\n", level);

				break;
			}
			case 44:
			{
				printf("input value of volume level(0 ~ 255, -1:back):");
				ret = getInputIntVal();
				if (ret < 0)
					break;
				else if (ret > 255)
					ret = 255;

				level = (uint8_t)ret;

				ret = QAUDIO_SetVoiceVolume(&hd, level);
				if (ret < 0)
					printf("QAUDIO_SetVoiceVolume error:%d\n", ret);
				else
					printf("QAUDIO_SetVoiceVolume: success\n");

				break;
			}
			case 45:
			{
				ret = QAUDIO_GetVoiceTxMute(&hd, &enable);
				if (ret < 0)
					printf("QAUDIO_GetVoiceTxMute error:%d\n", ret);
				else
					printf("Voice Tx mute state:%s\n", (enable == 0)?"Disabled":"Enabled");

				break;
			}
			case 46:
			{
				printf("input value to enable voice Tx mute(e.g. 0:disable 1:enable -1:back):");
				enable = getInputIntVal();
				if (enable < 0)
					break;

				ret = QAUDIO_SetVoiceTxMute(&hd, enable);
				if (ret < 0)
					printf("QAUDIO_SetVoiceTxMute error:%d\n", ret);
				else
					printf("Voice Tx mute:%s\n", (enable == 0)?"Disabled":"Enabled");

				break;
			}
			case 47:
			{
				ret = QAUDIO_GetVoiceRxMute(&hd, &enable);
				if (ret < 0)
					printf("QAUDIO_GetVoiceRxMute error:%d\n", ret);
				else
					printf("Voice Rx mute state:%s\n", (enable == 0)?"Disabled":"Enabled");

				break;
			}
			case 48:
			{
				printf("input value to enable voice Rx mute(e.g. 0:disable 1:enable -1:back):");
				enable = getInputIntVal();
				if (enable < 0)
					break;

				ret = QAUDIO_SetVoiceRxMute(&hd, enable);
				if (ret < 0)
					printf("QAUDIO_SetVoiceRxMute error:%d\n", ret);
				else
					printf("Voice Rx mute:%s\n", (enable == 0)?"Disabled":"Enabled");

				break;
			}
			case 49:
			{
				ret = QAUDIO_GetHandsFree(&hd, &enable);
				if (ret < 0)
					printf("QAUDIO_GetHandsFree error:%d\n", ret);
				else
					printf("Hands-Free state::%s\n", (enable == 0)?"Disabled":"Enabled");

				break;
			}
			case 50:
			{
				printf("input value to enable Hands-Free(e.g. 0:disable 1:enable -1:back):");
				enable = getInputIntVal();
				if (enable < 0)
					break;

				ret = QAUDIO_SetHandsFree(&hd, enable);
				if (ret < 0)
					printf("QAUDIO_SetHandsFree error:%d\n", ret);
				else
					printf("Hands-Free:%s\n", (enable == 0)?"Disabled":"Enabled");

				break;
			}
			case 51:
			{
				ret = QAUDIO_AddVoiceVolume(&hd, 5);
				if (ret < 0)
					printf("QAUDIO_AddVoiceVolume error:%d\n", ret);
				else
					printf("QAUDIO_AddVoiceVolume success\n");

				break;
			}
			case 52:
			{
				ret = QAUDIO_DecVoiceVolume(&hd, 5);
				if (ret < 0)
					printf("QAUDIO_DecVoiceVolume error:%d\n", ret);
				else
					printf("QAUDIO_DecVoiceVolume success\n");

				break;
			}
			case 90://play WAV file
			{
				ret = QAUDIO_OpenMusicFile(&hd, "/etc/mmi/qualsound.wav", FILE_WAV);
				if (ret < 0) {
					printf("QAUDIO_OpenMusicFile error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_OpenMusicFile: success\n");

				dev = AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
				ret = QAUDIO_OpenOutputDevice(&hd, dev, AUDIO_OUTPUT_FLAG_FAST);
				if (ret < 0) {
					printf("QAUDIO_OpenOutputDevice error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_OpenOutputDevice: success\n");

				level = 220; //0 < level <= 255
				ret = QAUDIO_SetOutputVolume(&hd, level);
				if (ret < 0)
					printf("QAUDIO_SetOutputVolume error:%d\n", ret);
				else
					printf("QAUDIO_SetOutputVolume: success\n");

				printf("1: play music\n");
				ret = QAUDIO_PlayMusic(&hd, MODE_BLOCK);
				if (ret < 0) {
					printf("QAUDIO_PlayMusic error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_PlayMusic: success\n");

				level = 5; //Increase output volume
				ret = QAUDIO_AddOutputVolume(&hd, level);
				if (ret < 0)
					printf("QAUDIO_SetOutputVolume error:%d\n", ret);
				else
					printf("change the volume success\n");

				printf("2: play music\n");
				ret = QAUDIO_PlayMusic(&hd, MODE_BLOCK);
				if (ret < 0)
					printf("QAUDIO_PlayMusic error:%d\n", ret);

				printf("3: play music non-block\n");
				ret = QAUDIO_PlayMusic(&hd, MODE_NONBLOCK);
				if (ret < 0)
					printf("QAUDIO_PlayMusic error:%d\n", ret);

				printf("sleep 1 second\n");
				sleep(1);//play music for 1 second

				ret = QAUDIO_StopMusic(&hd);
				if (ret < 0)
					printf("QAUDIO_StopMusic error:%d\n", ret);
				else
					printf("QAUDIO_StopMusic success\n");

				ret = QAUDIO_CloseMusicFile(&hd);
				if (ret < 0)
					printf("QAUDIO_CloseMusicFile error:%d\n", ret);
				else
					printf("QAUDIO_CloseMusicFile: success\n");

				ret = QAUDIO_CloseOutputDevice(&hd);
				if (ret < 0)
					printf("QAUDIO_CloseOutputDevice error:%d\n", ret);
				else
					printf("QAUDIO_CloseOutputDevice success\n");

				break;
			}
			case 91://play WAV file
			{
				dev =  AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_WIRED_HEADSET;//u can delete some one
				flags = AUDIO_OUTPUT_FLAG_FAST;

				ret = QAUDIO_SetOutputFlags(&hd, dev, flags);
				if (ret < 0) {
					printf("QAUDIO_SetOutputFlags error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_SetOutputFlags: success\n");

				format = AUDIO_FORMAT_PCM_16_BIT;
				rate = 44100;
				channels = 2;
				ret = QAUDIO_SetOutputConfig(&hd, format, rate, channels);
				if (ret < 0) {
					printf("QAUDIO_SetOutputConfig error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_SetOutputConfig: success\n");

				ret = QAUDIO_OpenMusicFile(&hd, "/etc/mmi/qualsound.wav", FILE_WAV);
				if (ret < 0) {
					printf("QAUDIO_OpenMusicFile error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_OpenMusicFile: success\n");

				level = 200; //0 < level <= 255
				ret = QAUDIO_SetOutputVolume(&hd, level);
				if (ret < 0)
					printf("QAUDIO_SetOutputVolume error:%d\n", ret);
				else
					printf("QAUDIO_SetOutputVolume: success\n");

				for (i = 0; i < 3; i++) {
					printf("start playing:%d\n", i);
					ret = QAUDIO_PlayMusic(&hd, MODE_BLOCK);
					if (ret < 0)
						printf("QAUDIO_PlayMusic error:%d\n", ret);
				}

				ret = QAUDIO_CloseMusicFile(&hd);
				if (ret < 0)
					printf("QAUDIO_CloseMusicFile error:%d\n", ret);
				else
					printf("QAUDIO_CloseMusicFile: success\n");

				break;
			}
			case 92: //play MP3 file
			{
				ret = QAUDIO_OpenMusicFile(&hd, "/etc/misc/soundlib/sound.mp3", FILE_MP3);
				if (ret < 0) {
					printf("QAUDIO_OpenMusicFile error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_OpenMusicFile: success\n");

				format = AUDIO_FORMAT_MP3;
				rate = 44100;
				channels = 2;
				ret = QAUDIO_SetOutputConfig(&hd, format, rate, channels);
				if (ret < 0) {
					printf("QAUDIO_SetOutputConfig error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_SetOutputConfig: success\n");

				dev = AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
				flags = AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD | AUDIO_OUTPUT_FLAG_DIRECT;

				ret = QAUDIO_OpenOutputDevice(&hd, dev, flags);
				if (ret < 0) {
					printf("QAUDIO_OpenOutputDevice error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_OpenOutputDevice: success\n");

				level = 200; //0 < level <= 255
				ret = QAUDIO_SetOutputVolume(&hd, level);
				if (ret < 0)
					printf("QAUDIO_SetOutputVolume error:%d\n", ret);
				else
					printf("QAUDIO_SetOutputVolume: success\n");

				printf("start playing MP3\n");
				ret = QAUDIO_PlayMusic(&hd, MODE_BLOCK);
				if (ret < 0) {
					printf("QAUDIO_PlayMusic error:%d\n", ret);
					break;
				}

				printf("sleep 3 second\n");
				sleep(3);

				printf("start playing MP3, non-block\n");
				ret = QAUDIO_PlayMusic(&hd, MODE_NONBLOCK);
				if (ret < 0)
					printf("QAUDIO_PlayMusic error:%d\n", ret);

				printf("sleep 3 second\n");
				sleep(3);//play music for 3 second

				printf("pause playing MP3...(3s)\n");
				ret = QAUDIO_PausePlayback(&hd);
				if (ret < 0)
					printf("QAUDIO_PausePlayback error:%d\n", ret);
				else
					printf("QAUDIO_PausePlayback: success\n");

				printf("sleep 3 second\n");
				sleep(3);//pause music for 3 second

				printf("resume playing MP3\n");
				ret = QAUDIO_ResumePlayback(&hd);
				if (ret < 0)
					printf("QAUDIO_ResumePlayback error:%d\n", ret);
				else
					printf("QAUDIO_ResumePlayback: success\n");

				printf("sleep 8 second\n");
				sleep(8);//waitfor

				ret = QAUDIO_CloseMusicFile(&hd);
				if (ret < 0)
					printf("QAUDIO_CloseMusicFile error:%d\n", ret);
				else
					printf("QAUDIO_CloseMusicFile: success\n");

				ret = QAUDIO_CloseOutputDevice(&hd);
				if (ret < 0)
					printf("QAUDIO_CloseOutputDevice error:%d\n", ret);
				else
					printf("QAUDIO_CloseOutputDevice: success\n");
				break;
			}
			case 93:
			{
				int offset = 0;
				size_t size = 0;
				size_t buffsize = 0;
				size_t bytes_wanted = 0;
				size_t bytes_remaining = 0;
				void *buff = NULL;
				WavFile *pwav = NULL;
				format = AUDIO_FORMAT_PCM_16_BIT;
				rate = 44100;
				size_t sample_size = 2;
				channels = 2;

				ret = process_wav_file(&pwav, "/etc/misc/soundlib/busy_tone.wav", &format, &sample_size, &channels, &rate);
				if(ret < 0) {
					printf("process_wav_file error:%d\n", ret);
					break;
				} else
					printf("process_wav_file: success\n");

				ret = QAUDIO_SetOutputConfig(&hd, format, rate, channels);
				if (ret < 0) {
					printf("QAUDIO_SetOutputConfig error:%d\n", ret);
					wav_close(pwav);
					pwav = NULL;
					break;
				} else
					printf("QAUDIO_SetOutputConfig: success\n");

				dev = AUDIO_DEVICE_OUT_SPEAKER;
				flags = AUDIO_OUTPUT_FLAG_FAST;

				ret = QAUDIO_OpenOutputDevice(&hd, dev, flags);
				if (ret < 0) {
					printf("QAUDIO_OpenOutputDevice error:%d\n", ret);
					wav_close(pwav);
					pwav = NULL;
					break;
				} else
					printf("QAUDIO_OpenOutputDevice: success\n");

				ret = QAUDIO_GetOutputBuffSize(&hd, &buffsize);
				if (ret < 0) {
					printf("QAUDIO_GetOutputBuffSize error:%d\n", ret);
					wav_close(pwav);
					pwav = NULL;
					break;
				} else
					printf("QAUDIO_GetOutputBuffSize buffer size[%d]\n", buffsize);

				buff = (void *)malloc(buffsize);
				if (buff == NULL) {
					printf("malloc failed\n");
					wav_close(pwav);
					pwav = NULL;
					break;
				}

				printf("Begin of playing wave file\n");
				for (i = 0; i < 10; i++) {
					printf("The number of loop playing:%d\n", i);
					do {
						if (bytes_remaining == 0) {
							bytes_wanted = buffsize/channels/sample_size;
							size = wav_read(pwav, buff, bytes_wanted);
							if (size == 0) {
								break;
							}

							offset = 0;
							bytes_remaining = size * channels * sample_size;
						}

						ret = QAUDIO_WriteOutputStream(&hd, (char *)(buff + offset), bytes_remaining);
						//printf("bytes_remaining[%d] ret[%d]\n", bytes_remaining, ret);
						if (ret < 0) {
							printf("QAUDIO_WriteOutputStream error:%d\n", ret);
							break;
						} else {
							offset = ret;
							bytes_remaining -= ret;
						}

					} while(size > 0);

					wav_rewind(pwav);//rewind the wav file
					usleep(500000);//500ms
				}
				printf("End of playing wave file\n");

				ret = QAUDIO_CloseOutputDevice(&hd);
				if (ret < 0)
					printf("QAUDIO_CloseOutputDevice error:%d\n", ret);
				else
					printf("QAUDIO_CloseOutputDevice: success\n");

				if (buff) {
					free(buff);
					buff = NULL;
				}

				if (pwav) {
					wav_close(pwav);
					pwav = NULL;
				}

				break;
			}
			case 94:
			{
				ret = QAUDIO_SetInputFlags(&hd, AUDIO_DEVICE_IN_BUILTIN_MIC,
						AUDIO_INPUT_FLAG_FAST);
				if (ret < 0) {
					printf("QAUDIO_SetInputFlags error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_SetInputFlags: success\n");

				ret = QAUDIO_SetInputConfig(&hd, AUDIO_FORMAT_PCM_16_BIT, 8000, 1);
				if (ret < 0) {
					printf("QAUDIO_SetInputConfig error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_SetInputConfig: success\n");

				printf("start record... (10s)\n");
				ret = QAUDIO_StartRecordWav(&hd, "/data/record.wav", 10.0);//10s
				if (ret < 0) {
					printf("QAUDIO_StartRecordWav error:%d\n", ret);
					break;
				} else
					printf("Recording end\n");

				ret = QAUDIO_OpenMusicFile(&hd, "/data/record.wav", FILE_WAV);
				if (ret < 0) {
					printf("QAUDIO_OpenMusicFile error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_OpenMusicFile: success\n");

				dev =  AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_WIRED_HEADSET;//u can delete some one
				flags = AUDIO_OUTPUT_FLAG_DEEP_BUFFER;

				ret = QAUDIO_SetOutputFlags(&hd, dev, flags);
				if (ret < 0) {
					printf("QAUDIO_SetOutputFlags error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_SetOutputFlags: success\n");

				format = AUDIO_FORMAT_PCM_16_BIT;
				rate = 8000;
				channels = 1;
				ret = QAUDIO_SetOutputConfig(&hd, format, rate, channels);
				if (ret < 0) {
					printf("QAUDIO_SetOutputConfig error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_SetOutputConfig: success\n");


				ret = QAUDIO_PlayMusic(&hd, MODE_BLOCK);
				if (ret < 0) {
					printf("QAUDIO_PlayMusic error:%d\n", ret);
				} else
					printf("QAUDIO_PlayMusic: success\n");

				ret = QAUDIO_CloseMusicFile(&hd);
				if (ret < 0)
					printf("QAUDIO_CloseMusicFile error:%d\n", ret);
				else
					printf("QAUDIO_CloseMusicFile: success\n");

				ret = QAUDIO_CloseOutputDevice(&hd);
				if (ret < 0)
					printf("QAUDIO_CloseOutputDevice error:%d\n", ret);
				else
					printf("QAUDIO_CloseOutputDevice: success\n");

				break;
			}
			case 95:
			{
				ret = QAUDIO_SetInputFlags(&hd, AUDIO_DEVICE_IN_WIRED_HEADSET,
						AUDIO_INPUT_FLAG_FAST);
				if (ret < 0) {
					printf("QAUDIO_SetInputFlags error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_SetInputFlags: success\n");

				ret = QAUDIO_SetInputConfig(&hd, AUDIO_FORMAT_PCM_16_BIT, 8000, 1);
				if (ret < 0) {
					printf("QAUDIO_SetInputConfig error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_SetInputConfig: success\n");

				printf("start record... (10s)\n");
				ret = QAUDIO_StartRecordWav(&hd, "/data/record.wav", 10.0);//10s
				if (ret < 0) {
					printf("QAUDIO_StartRecordWav error:%d\n", ret);
					break;
				} else
					printf("Recording end\n");

				ret = QAUDIO_OpenMusicFile(&hd, "/data/record.wav", FILE_WAV);
				if (ret < 0) {
					printf("QAUDIO_OpenMusicFile error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_OpenMusicFile: success\n");

				ret = QAUDIO_PlayMusic(&hd, MODE_BLOCK);
				if (ret < 0) {
					printf("QAUDIO_PlayMusic error:%d\n", ret);
				} else
					printf("QAUDIO_PlayMusic: success\n");

				ret = QAUDIO_CloseMusicFile(&hd);
				if (ret < 0)
					printf("QAUDIO_CloseMusicFile error:%d\n", ret);
				else
					printf("QAUDIO_CloseMusicFile: success\n");

				break;
			}
			case 96:
			{
				size_t buffsize = 0;
				int offset = 0;
				size_t size = 0;
				size_t bytes_wanted = 0;
				size_t bytes_remaining = 0;
				void *buff = NULL;
				WavFile *pwav = NULL;
				int format = AUDIO_FORMAT_PCM_16_BIT;
				int rate = 44100;
				size_t sample_size = 2;
				int channels = 2;
				pthread_t thread;

				ret = pthread_create(&thread, NULL, play_wav_file_with_stream, NULL);
				if(ret != 0) {
					printf("pthread_create err:%d\n", ret);
					goto over;
				}

				ret = process_wav_file(&pwav, "/etc/misc/soundlib/busy_tone.wav", &format, &sample_size, &channels, &rate);
				if(ret < 0) {
					printf("process_wav_file error:%d\n", ret);
					goto over;
				} else
					printf("process_wav_file: success\n");

				ret = QAUDIO_SetOutputConfig(&hd, format, rate, channels);
				if (ret < 0) {
					printf("QAUDIO_SetOutputConfig error:%d\n", ret);
					goto over;
				} else
					printf("QAUDIO_SetOutputConfig: success\n");

				dev = AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_WIRED_HEADPHONE;;
				flags = AUDIO_OUTPUT_FLAG_FAST;

				ret = QAUDIO_OpenOutputDevice(&hd, dev, flags);
				if (ret < 0) {
					printf("QAUDIO_OpenOutputDevice error:%d\n", ret);
					goto over;
				} else
					printf("QAUDIO_OpenOutputDevice: success\n");

				level = 200; //0 < level <= 255
				ret = QAUDIO_SetOutputVolume(&hd, level);
				if (ret < 0)
					printf("QAUDIO_SetOutputVolume error:%d\n", ret);
				else
					printf("QAUDIO_SetOutputVolume: success\n");

				ret = QAUDIO_GetOutputBuffSize(&hd, &buffsize);
				if (ret < 0) {
					printf("QAUDIO_GetOutputBuffSize error:%d\n", ret);
					goto over;
				} else
					printf("QAUDIO_GetOutputBuffSize buffer size[%d]\n", buffsize);

				buff = (void *)malloc(buffsize);
				if (buff == NULL) {
					printf("malloc failed\n");
					goto over;
				}

				pthread_mutex_lock(&s_stateMutex);

				printf("Begin of playing wave file\n");
				for (i = 0; i < 3; i++) {
					printf("The number of loop playing:%d\n", i);
					do {
						if (bytes_remaining == 0) {
							bytes_wanted = buffsize/channels/sample_size;
							size = wav_read(pwav, buff, bytes_wanted);
							if (size == 0) {
								continue;
							}

							offset = 0;
							bytes_remaining = size * channels * sample_size;
						}

						ret = QAUDIO_WriteOutputStream(&hd, (char *)(buff + offset), bytes_remaining);
						//printf("bytes_remaining[%d] ret[%d]\n", bytes_remaining, ret);
						if (ret < 0) {
							printf("QAUDIO_WriteOutputStream error:%d\n", ret);
							break;
						} else {
							offset = ret;
							bytes_remaining -= ret;
						}

					} while(size > 0);

					wav_rewind(pwav);//rewind the wav file
					usleep(500000);//500ms
				}
				printf("End of playing wave file\n");

				pthread_mutex_unlock(&s_stateMutex);
over:
				if(thread)
				pthread_join(thread, NULL);

				ret = QAUDIO_CloseOutputDevice(&hd);
				if (ret < 0)
					printf("QAUDIO_CloseOutputDevice error:%d\n", ret);
				else
					printf("QAUDIO_CloseOutputDevice: success\n");

				if (buff) {
					free(buff);
					buff = NULL;
				}

				if (pwav) {
					wav_close(pwav);
					pwav = NULL;
				}

				break;
			}
			case 97:
			{
				int offset = 0;
				size_t size = 0;
				size_t buffsize = 0;
				size_t bytes_wanted = 0;
				size_t bytes_remaining = 0;
				void *buff = NULL;
				WavFile *pwav = NULL;
				format = AUDIO_FORMAT_PCM_16_BIT;
				rate = 44100;
				size_t sample_size = 2;
				channels = 2;

				WavFile *pwav2 = NULL;
				int format2 = AUDIO_FORMAT_PCM_16_BIT;
				int rate2 = 44100;
				size_t sample_size2 = 2;
				int channels2 = 2;

				//HPHL
				ret = process_wav_file(&pwav, "/etc/misc/soundlib/Backroad.wav",
						&format, &sample_size, &channels, &rate);
				if(ret < 0) {
					printf("process_wav_file error:%d\n", ret);
					break;
				} else
					printf("process_wav_file: success\n");

				//HPHR and SPK
				ret = process_wav_file(&pwav2, "/etc/mmi/qualsound.wav",
						&format2, &sample_size2, &channels2, &rate2);
				if(ret < 0) {
					printf("process_wav_file2 error:%d\n", ret);
					goto over2;
				} else
					printf("process_wav_file2: success\n");

				ret = QAUDIO_SetOutputConfig(&hd, format, rate, channels);
				if (ret < 0) {
					printf("QAUDIO_SetOutputConfig error:%d\n", ret);
					goto over2;
				} else
					printf("QAUDIO_SetOutputConfig: success\n");

				dev = AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
				flags = AUDIO_OUTPUT_FLAG_FAST;

				ret = QAUDIO_OpenOutputDevice(&hd, dev, flags);
				if (ret < 0) {
					printf("QAUDIO_OpenOutputDevice error:%d\n", ret);
					goto over2;
				} else
					printf("QAUDIO_OpenOutputDevice: success\n");

				ret = QAUDIO_SetOutputVolume(&hd, 200);
				if (ret < 0) {
					printf("QAUDIO_SetOutputVolume error:%d\n", ret);
					goto over2;
				}
				else
					printf("QAUDIO_SetOutputVolume: success\n");

				ret = QAUDIO_GetOutputBuffSize(&hd, &buffsize);
				if (ret < 0) {
					printf("QAUDIO_GetOutputBuffSize error:%d\n", ret);
					goto over2;
				} else
					printf("QAUDIO_GetOutputBuffSize buffer size[%d]\n", buffsize);

				buff = (void *)malloc(buffsize);
				if (buff == NULL) {
					printf("malloc failed\n");
					goto over2;
				}

				printf("Begin of playing wave file\n");

				do {
					if (bytes_remaining == 0) {
						bytes_wanted = buffsize/channels/sample_size;
						size = wav_read_LR_channels(pwav, pwav2, buff, bytes_wanted);
						if (size == 0) {
							break;
						}

						offset = 0;
						bytes_remaining = size * channels * sample_size;
					}

					ret = QAUDIO_WriteOutputStream(&hd, (char *)(buff + offset), bytes_remaining);
					//printf("bytes_remaining[%d] ret[%d]\n", bytes_remaining, ret);
					if (ret < 0) {
						printf("QAUDIO_WriteOutputStream error:%d\n", ret);
						break;
					} else {
						offset = ret;
						bytes_remaining -= ret;
					}

				} while(size > 0);

				usleep(500000);//500ms

				printf("End of playing wave file\n");

over2:
				ret = QAUDIO_CloseOutputDevice(&hd);
				if (ret < 0)
					printf("QAUDIO_CloseOutputDevice error:%d\n", ret);
				else
					printf("QAUDIO_CloseOutputDevice: success\n");

				if (buff) {
					free(buff);
					buff = NULL;
				}

				if (pwav) {
					wav_close(pwav);
					pwav = NULL;
				}

				if (pwav2) {
					wav_close(pwav2);
					pwav2 = NULL;
				}

				break;
			}
			case 98:
			{
				file_e type = FILE_MP3;

				printf("input the path of MP3 file(-1: back):");
				ret = getInputString(buf, sizeof(buf));
				if (strncmp(buf, "-1", 2) == 0)
					break;
				else if (ret < 3)
					snprintf(buf, sizeof(buf), "%s", "/etc/misc/soundlib/sound.mp3");

				ret = QAUDIO_OpenMusicFile(&hd, buf, type);
				if (ret < 0)
					printf("QAUDIO_OpenMusicFile error:%d\n", ret);
				else
					printf("QAUDIO_OpenMusicFile: success\n");

				ret = QAUDIO_PlayMusic(&hd, 1);
				if (ret < 0)
					printf("QAUDIO_PlayMusic error:%d\n", ret);
				else
					printf("QAUDIO_PlayMusic: success\n");
				printf("Playing music on speaker for 20 seconds...\n");
				sleep(20);

				printf("switch output device on headphone\n");
				dev = AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
				flags = AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD | AUDIO_OUTPUT_FLAG_DIRECT;

				ret = QAUDIO_SwitchOutputDevice(&hd, dev, flags);
				if (ret < 0) {
					printf("QAUDIO_SwitchOutputDevice error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_SwitchOutputDevice: success\n");

				printf("Playing music on headphone for 20 seconds...\n");
				sleep(20);

				printf("switch output device on Speaker\n");
				dev = AUDIO_DEVICE_OUT_SPEAKER;

				ret = QAUDIO_SwitchOutputDevice(&hd, dev, flags);
				if (ret < 0) {
					printf("QAUDIO_SwitchOutputDevice error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_SwitchOutputDevice: success\n");

				printf("Playing music on speaker for 20 seconds...\n");
				sleep(20);

				ret = QAUDIO_StopMusic(&hd);
				if (ret < 0)
					printf("QAUDIO_StopMusic error:%d\n", ret);
				else
					printf("QAUDIO_StopMusic: success\n");

				ret = QAUDIO_CloseOutputDevice(&hd);
				if (ret < 0)
					printf("QAUDIO_CloseOutputDevice error:%d\n", ret);
				else
					printf("QAUDIO_CloseOutputDevice: success\n");

				break;
			}
			case 99:
			{
				size_t buffsize = 0;
				int offset = 0;
				size_t size = 0;
				size_t bytes_remaining = 0;
				void *buff = NULL;
				pthread_t thread;
				FILE *outputfp;
				file_e type = FILE_MP3;

				printf("input the path of MP3 file(-1: back):");
				ret = getInputString(buf, sizeof(buf));
				if (strncmp(buf, "-1", 2) == 0)
					break;
				else if (ret < 3)
					snprintf(buf, sizeof(buf), "%s", "/etc/misc/soundlib/sound.mp3");

				outputfp = fopen(buf, "r");
				if (outputfp == NULL) {
					printf("Cannot open music file\n");
					break;
				}

				pthread_mutex_init(&s_stateMutex, NULL);
				pthread_mutex_lock(&s_stateMutex);

				ret = pthread_create(&thread, NULL, switch_device, (void*)&hd);
				if (ret != 0) {
					printf("pthread_create err:%d\n", ret);
					goto over3;
				}

				//Set up MP3 music file configuration data
				format = AUDIO_FORMAT_MP3;
				rate = 48000;
				channels = 2;
				ret = QAUDIO_SetOutputConfig(&hd, format, rate, channels);
				if (ret < 0) {
					printf("QAUDIO_SetOutputConfig error:%d\n", ret);
					break;
				} else
					printf("QAUDIO_SetOutputConfig: success\n");

				dev = AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
				flags = AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD | AUDIO_OUTPUT_FLAG_DIRECT;

				ret = QAUDIO_SwitchOutputDevice(&hd, dev, flags);
				if (ret < 0) {
					printf("QAUDIO_SwitchOutputDevice error:%d\n", ret);
					goto over3;
				} else
					printf("QAUDIO_SwitchOutputDevice: success\n");

				ret = QAUDIO_GetOutputBuffSize(&hd, &buffsize);
				if (ret < 0) {
					printf("QAUDIO_GetOutputBuffSize error:%d\n", ret);
					goto over3;
				} else
					printf("QAUDIO_GetOutputBuffSize buffer size[%d]\n", buffsize);

				buff = (void *)malloc(buffsize);
				if (buff == NULL) {
					printf("malloc failed\n");
					goto over3;
				}

				pthread_mutex_unlock(&s_stateMutex);

				stop_play = 0;
				pause_play = 0;

				printf("Playing music on headphone for 20 seconds...\n");
				do {
					if (bytes_remaining == 0) {
						if (stop_play == 1) {
							goto over3;
						}

						while(pause_play == 1) {
							usleep(10000);

							if (stop_play == 1) {//stop on
								goto over3;
							}

							if (pause_play != 1) {//pause off
								//back to the missing position, do not change -21300 to a different value
								ret = fseek(outputfp, -213000, SEEK_CUR);
								if (ret != 0) {
									printf("fseek error:%d\n",ret);
								}

								break;
							}
						}

						size = fread(buff, 1, buffsize/4, outputfp);
						if (size == 0) {
							continue;
						}

						offset = 0;
						bytes_remaining = size;
					}

					ret = QAUDIO_WriteOutputStream(&hd, (char *)(buff + offset), bytes_remaining);
					//printf("bytes_remaining[%d] ret[%d]\n", bytes_remaining, ret);
					if (ret < 0) {
						printf("QAUDIO_WriteOutputStream error:%d\n", ret);
						break;
					} else {
						offset = ret;
						bytes_remaining -= ret;
					}

				} while(size > 0);

				usleep(500000);//500ms
				printf("End of playing MP3 file\n");

over3:
				pthread_mutex_destroy(&s_stateMutex);

				if(thread)
					pthread_join(thread, NULL);

				ret = QAUDIO_StopMusic(&hd);
				if (ret < 0)
					printf("QAUDIO_StopMusic error:%d\n", ret);
				else
					printf("QAUDIO_StopMusic: success\n");

				ret = QAUDIO_CloseMusicFile(&hd);
				if (ret < 0)
					printf("QAUDIO_CloseMusicFile error:%d\n", ret);
				else
					printf("QAUDIO_CloseMusicFile: success\n");

				ret = QAUDIO_CloseOutputDevice(&hd);
				if (ret < 0)
					printf("QAUDIO_CloseOutputDevice error:%d\n", ret);
				else
					printf("QAUDIO_CloseOutputDevice: success\n");

				if (buff) {
					free(buff);
					buff = NULL;
				}

				break;
			}
			default:
			{
				char ch = 0;
				printf("Input command index unknown:%d\n", cmdIdx);
				while((ch = getchar()) !='\n' && ch != EOF);
				break;
			}
		}
	}

	return 0;
} /* ----- End of main() ----- */
