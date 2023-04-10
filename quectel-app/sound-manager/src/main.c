/*
 *      Copyright:  (C) 2020 quectel
 *                  All rights reserved.
 *       Filename:  main.c
 *    Description:  sound-Manager
 *        Version:  1.0.0(20200930)
 *         Author:  Peeta/fulinux <peeta.chen@quectel.com>
 *      ChangeLog:  1, Release initial version on 20200930
 *					2, using libqaudio APIs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <pthread.h>

#include <qaudio.h>
#include <ql_misc.h>
#include <RilPublic.h>
#include <fileparser/iniparser.h>
#include <fileparser/hexparser.h>

#include "wav.h"
#include "linked_list.h"

#define MSMAUDIO_DEBUG 1
#ifdef MSMAUDIO_DEBUG

#ifndef LOG_TAG
#define LOG_TAG "soundmgr"
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

#define SOUNDLIB_PATH "/etc/misc/soundlib/"
#define SOUNDMGR_CONFIG  "/etc/sound_manager.ini"

static int handle = 0;
static int qlril_exit = 0;
static int sndHandle = 0;
static int stop_sound_play = 1;
static int sound_play_state = 0;  //0 停止 1 播放 2 播放线程结束
static int have_calls = 0;
static int handsfree = 1;
static int call_active = 0;
static int calls_sum = 0;
static int calls_sum_old = 0;
static int holding2active = 0;
static int mute_state = 0;
static int mute_state_old = 1;
static int soundmgr_start = 0;
static int operator_mccmnc = 0;

static pthread_mutex_t s_stateMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_audioMutex = PTHREAD_MUTEX_INITIALIZER;

struct calls_state {
	int index;
	int state;
	int slotId;
	char number[100];
};

struct sound_strategy {
	int incall;
	int strategy;
	char wavfile[100];
};

struct input_event {
	struct timeval time;
	uint16_t type;
	uint16_t code;
	long value;
};

list_node_t *state_list = NULL;

static int play_wav_file(char *name, int mode, int incall)
{
	int ret = 0;
	uint32_t dev, flags;
	int format = AUDIO_FORMAT_PCM_16_BIT;
	int rate = 44100;
	int channels = 2;
	int bit = 16;
	char buff[100] = {0};

	if (name == NULL) {
		QLOGE ("Error: parameter name is NULL");
		return -1;
	}

	//dev =  AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
	if (handsfree != 0)
		dev =  AUDIO_DEVICE_OUT_SPEAKER;
	else {
		//dev = AUDIO_DEVICE_OUT_EARPIECE;
		dev =  AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
	}

	//flags = AUDIO_OUTPUT_FLAG_DIRECT;
	flags = AUDIO_OUTPUT_FLAG_FAST;

	snprintf(buff, sizeof(buff), "%s%s", SOUNDLIB_PATH, name);

	ret = QAUDIO_OpenMusicFile(&sndHandle, buff, FILE_WAV);
	if (ret < 0) {
		QLOGE("QAUDIO_OpenMusicFile error:%d", ret);
		return -2;
	}

	ret = QAUDIO_SetOutputFlags(&sndHandle, dev, flags);
	if (ret < 0) {
		QLOGE("QAUDIO_SetOutputFlags error:%d", ret);
		return -3;
	}

	ret = QAUDIO_SetOutputConfig(&sndHandle, AUDIO_FORMAT_PCM_16_BIT, 44100, 2);
	if (ret < 0) {
		QLOGE("QAUDIO_SetOutputConfig error:%d", ret);
		return -4;
	}

	ret = QAUDIO_PlayMusic(&sndHandle, mode);
	if (ret < 0)
		QLOGE("QAUDIO_PlayMusic error:%d", ret);

	return 0;
}

static void *play_music_thread(void *args)
{
	int i = 0;
	int j = 0;
	int ret = 0;
	int offset = 0;
	int strategy = 0;
	int rate = 44100;
	int channels = 2;
	int format = 0;
	size_t size = 0;
	size_t buffsize = 0;
	size_t sample_size = 2;
	size_t bytes_wanted = 0;
	size_t bytes_remaining = 0;
	uint32_t dev, flags;
	void *buff = NULL;
	char wavfile[100] = {0};
	struct sound_strategy *p = (struct sound_strategy *)args;
	WavFile *pwav = NULL;

	if (sndHandle == 0)
		return NULL;

	strategy = p->strategy;
	snprintf(wavfile, sizeof(wavfile), "%s%s", SOUNDLIB_PATH, p->wavfile);
	if (strlen(wavfile) <= 0) {
		QLOGD("wavfile is invalid");
		return NULL;
	}

	pwav = wav_open(wavfile, WAV_OPEN_READ);
	if (pwav == NULL) {
		QLOGE("wav_open error:%d", ret);
		return NULL;
	}

	format = wav_get_format(pwav);
	sample_size = wav_get_sample_size(pwav);
	if (format == WAV_FORMAT_PCM && sample_size == 1)//16bit pcm
		format = AUDIO_FORMAT_PCM_8_BIT;
	else if (format == WAV_FORMAT_PCM && sample_size == 4)//16bit pcm
		format = AUDIO_FORMAT_PCM_32_BIT;
	else if (format == WAV_FORMAT_IEEE_FLOAT)
		format = AUDIO_FORMAT_PCM_FLOAT;
	else
		format = AUDIO_FORMAT_PCM_16_BIT;

	QLOGI("wave file format code:0x%X, sample size:%d", format, sample_size);

	channels = wav_get_num_channels(pwav);
	if (channels < 1)
		channels = 1;
	else if (channels > 2)
		channels = 2;

	rate = wav_get_sample_rate(pwav);

	size = wav_get_length(pwav);

	ret = QAUDIO_SetOutputConfig(&sndHandle, format, rate, channels);
	if (ret < 0) {
		QLOGE("QAUDIO_SetOutputConfig error:%d", ret);
		goto fail1;
	}

	if (handsfree != 0)
		dev =  AUDIO_DEVICE_OUT_SPEAKER;
	else {
		//dev = AUDIO_DEVICE_OUT_EARPIECE;
		dev =  AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
	}

	if (strategy == 0)
		dev =  AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_WIRED_HEADPHONE;

	flags = AUDIO_OUTPUT_FLAG_FAST;
	//flags = AUDIO_OUTPUT_FLAG_DIRECT;
	ret = QAUDIO_OpenOutputDevice(&sndHandle, dev, flags);
	if (ret < 0) {
		QLOGE("QAUDIO_OpenOutputDevice error:%d", ret);
		goto fail1;
	}

	ret = QAUDIO_GetOutputBuffSize(&sndHandle, &buffsize);
	if (ret < 0 || buffsize == 0) {
		QLOGE("QAUDIO_OutputGetBuffSize error:%d", ret);
		goto fail2;
	}

	buff = (void *)malloc(buffsize);
	if (buff == NULL) {
		QLOGE("malloc failed");
		goto fail2;
	}

	stop_sound_play = 0; //播放
	sound_play_state = 1;

	QLOGD("Playing Begin");
	for (i = 0; i < 30 && stop_sound_play == 0; i++) {
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

			ret = QAUDIO_WriteOutputStream(&sndHandle, (char *)(buff + offset), bytes_remaining);
			//QLOGI("bytes_remaining[%d] ret[%d]\n", bytes_remaining, ret);
			if (ret < 0) {
				QLOGE("QAUDIO_WriteOutputStream error:%d", ret);
				break;
			} else {
				offset = ret;
				bytes_remaining -= ret;
			}
		} while(size > 0);

		if (stop_sound_play != 0)
			break;

		if (strategy == 3)
			break;

		wav_rewind(pwav);//rewind the wav file

		switch (strategy) {
		case 1:
		{
			static int times = 0;

			if ((++times % 2) == 1)
				usleep(350000);
			else
				for (j = 0; stop_sound_play == 0 && j < 20; j++)
					usleep(100000);

			break;
		}
		case 2:
		{
			for (j = 0; stop_sound_play == 0 && j < 40; j++)
				usleep(100000);

			break;
		}
		default:
			for (j = 0; stop_sound_play == 0 && i < 10 && j < 38; j++)
				usleep(50000);
			break;
		}
	}
	QLOGD("Playing End");

fail2:
	ret = QAUDIO_CloseOutputDevice(&sndHandle);
	if (ret < 0)
		QLOGE("QAUDIO_CloseOutputDevice error:%d", ret);

fail1:
	if (pwav) {
		wav_close(pwav);
		pwav = NULL;
	}

	stop_sound_play = 2;
	sound_play_state = 2; //播放线程结束，但是播放器还没有关闭

	return NULL;
}

static int play_music_start(void *args)
{
	int ret = 0;
	pthread_t play_music_tid = 0;

	ret = pthread_create(&play_music_tid, NULL,
			play_music_thread, (void *)args);
	if (ret != 0) {
		QLOGE ("Error: pthread_create");
		return -1;
	}

	return 0;
}

static int play_music_stop(void *args)
{
	int i = 0;
	int ret = 0;
	struct sound_strategy *p = (struct sound_strategy *)args;

	QLOGD("Enter");

	if (sndHandle == 0)
		return -1;

	stop_sound_play = 1;

	if (sound_play_state == 0 || sound_play_state == 2) {
		QLOGD("not playing\n");
	}

	if (sound_play_state == 1) {
		for(i = 0; i < 10 && sound_play_state != 2; i++) {
			ret = QAUDIO_StopMusic(&sndHandle);
			if (ret < 0)
				QLOGW("QAUDIO_StopMusic error:%d", ret);

			stop_sound_play = 1;
			usleep(50000);
		}
	}

	sound_play_state = 0;

	QLOGD("Exit");

	return 0;
}

static int prepare_voice_call(void)
{
	int ret = 0;
	uint32_t dev, flags;

	//dev =  AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_WIRED_HEADSET;
	if (handsfree != 0)
		dev =  AUDIO_DEVICE_OUT_SPEAKER;
	else {
		//dev = AUDIO_DEVICE_OUT_EARPIECE;
		dev =  AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
	}
	flags = AUDIO_OUTPUT_FLAG_PRIMARY;

	ret = QAUDIO_PrepareVoiceCall(&sndHandle, dev, flags);
	if (ret < 0) {
		QLOGE("QAUDIO_PrepareVoiceCall error:%d", ret);
		return -1;
	}

	return 0;
}

static void *sound_status_switch(void *args)
{
	int i = 0;
	int ret = 0;
	int new_index = -1;
	int old_index = -1;
	int new_state = -1;
	int old_state = -1;
	int slotId = 1;
	char dev[] = "/dev/smd8";
	char new_number[100] = {0};
	char old_number[100] = {0};

	struct calls_state *cstate = NULL;
	struct calls_state *pstate = NULL;
	struct sound_strategy ss;
	list_node_t *node = NULL;
	RIL_Call *calls = NULL;

	char *strCallState[7] = {"ACTIVE", "HOLDING", "DIALING",
		"ALERTING", "INCOMING", "WAITING", "END"};

	QLOGD("sound_status_switch thread stated");
	for (;;) {
		pthread_mutex_lock(&s_stateMutex);
		for (node = state_list; node != NULL; node = node->next) {
			cstate = (struct calls_state *)node->data;
			if (cstate == NULL)
				continue;

			new_index = cstate->index;
			new_state = cstate->state;
			slotId = cstate->slotId;
			memset(new_number, 0, sizeof(new_number));
			if (strlen(cstate->number) > 0) {
				snprintf(new_number, sizeof(new_number), "%s", cstate->number);
				QLOGD("Receivce new state:%d, numer:%s\n", new_state, new_number);
			}

			QLOGD("new_index[%d] new_state[%d]", new_index, new_state);
			free(cstate);
			node->data = NULL;
			list_remove(&state_list, node);
			break;
		}
		pthread_mutex_unlock(&s_stateMutex);

		if (call_active == 1) {
			ret = QLRIL_GetMute(&handle, &mute_state);
			if (mute_state != mute_state_old) {
				mute_state_old = mute_state;
				//set voice call mute on or off
				ret = QAUDIO_SetVoiceTxMute(&sndHandle, mute_state);
				if (ret < 0)
					QLOGE("QAUDIO_SetVoiceTxMute error:%d", ret);
			}
			usleep(1000);
		}

		if (old_state != new_state) {
			QLOGD("new_state[%d], old_state[%d]", new_state, old_state);
			QLOGD("calls_sum_old[%d], calls_sum[%d]", calls_sum_old, calls_sum);
			old_state = new_state;
		} else {
			if (state_list == NULL) {
				if (old_state == 4)
					sleep(1);
				else
					sleep(2);
			} else
				usleep(1000);
			continue;
		}

		QLOGD("new_index[%d], old_index[%d]", new_index, old_index);
		QLOGD("new_number[%s], old_number[%s]", new_number, old_number);
		QLOGD("holding2active[%d], call_active[%d]", holding2active, call_active);

		QLOGD("Call state[%s] Enter", strCallState[new_state]);

		switch(new_state) {
		case 0: //ACTIVE
		{
			if (calls_sum < 2)
				play_music_stop(NULL);

			if (call_active == 1) {
				old_index = new_index;
				snprintf(old_number, sizeof(old_number), "%s", new_number);
				break;
			}

			ret = QAUDIO_StartVoiceCall(&sndHandle);
			if (ret < 0)
				QLOGD("QAUDIO_StartVoiceCall error:%d", ret);

			ret = QAUDIO_SetHandsFree(&sndHandle, handsfree);
			if (ret < 0)
				QLOGD("QAUDIO_SetHandsFree error:%d", ret);

			call_active = 1;
			old_index = new_index;

			break;
		}
		case 1: //HOLDING
		{
			play_music_stop(NULL);
			if (holding2active != 0 && handle != 0) {
				ret = QLRIL_SwitchWaitingOrHoldingAndActive(&handle);
				QLOGD("QLRIL_SwitchWaitingOrHoldingAndActive return:%d", ret);
				holding2active = 0;
			}
			break;
		}
		case 2: //DIALING
		case 3: //ALERTING
		{
			char **opt = NULL;
			if (operator_mccmnc == 0) {
				play_music_stop((void *)&ss);

				ret = QLRIL_SetSimCardSlotId(&handle, slotId);
				if (ret < 0)
					QLOGE("QLRIL_SetSimCardSlotId error:%d", ret);
				else
					QLOGI("QLRIL_SetSimCardSlotId success slotId[%d]", slotId);

				ret = QLRIL_GetOperator(&handle, &opt);
				if (ret > 0) {
					if (opt[2]) {
						operator_mccmnc = atoi(opt[2]);
					}
					free(opt[0]);
					free(opt[1]);
					free(opt[2]);
					free(opt);
					opt = NULL;
				}

				if (operator_mccmnc == 46000) {
					ss.strategy = 2;
					snprintf(ss.wavfile, sizeof(ss.wavfile), "%s", "ring_back_tone.wav");
					play_music_start((void *)&ss);
				}
			}

			ret = QAUDIO_StartVoiceCall(&sndHandle);
			if (ret == 0) {
				ret = QAUDIO_SetHandsFree(&sndHandle, handsfree);
				if (ret < 0)
					QLOGD("QAUDIO_SetHandsFree error:%d", ret);
			} else
				QLOGD("QAUDIO_StartVoiceCall error:%d", ret);

			break;
		}
		case 4: //INCOMING
		{
			play_music_stop((void *)&ss);

			ret = QLRIL_SetSimCardSlotId(&handle, slotId);
			if (ret < 0)
				QLOGE("QLRIL_SetSimCardSlotId error:%d", ret);
			else
				QLOGI("QLRIL_SetSimCardSlotId success slotId[%d]", slotId);

			if (calls_sum > 1) { //busy tone, 450Hz, -10dBm0, play:350ms, stop:350ms, alternately
				ss.strategy = 1;
				snprintf(ss.wavfile, sizeof(ss.wavfile), "%s", "busy_tone.wav");
			} else {//telephony ring
				call_active = 0;
				ss.strategy = 0;
				snprintf(ss.wavfile, sizeof(ss.wavfile), "%s", "telephone_ring.wav");
			}

			play_music_start((void *)&ss);

			break;
		}
		case 5: //WAITING
		{
			play_music_stop((void *)&ss);
			if (call_active == 1) { //busy tone, 450Hz, -10dBm0, play:350ms, stop:350ms, alternately
				ss.incall = 1;
				ss.strategy = 1;
				snprintf(ss.wavfile, sizeof(ss.wavfile), "%s", "busy_tone.wav");
				play_music_start((void *)&ss);
			}

			break;
		}
		case 6: //END
		{
			if (call_active == 1) {
				ss.strategy = 1;
				play_music_stop((void *)&ss);
			} else
				play_music_stop(NULL);

			ret = QAUDIO_SetVoiceTxMute(&sndHandle, 0);
			if (ret < 0)
				QLOGE("QAUDIO_SetVoiceTxMute error:%d", ret);

			if (call_active == 1 && calls_sum <= 1) {
				call_active = 0;

				new_index = old_index = -1;
				memset(old_number, 0, sizeof(old_number));
				ss.incall = 0;
				ss.strategy = 3;
				snprintf(ss.wavfile, sizeof(ss.wavfile), "%s", "busy_tone.wav");
				play_music_start((void *)&ss);
				state_list = NULL;
				calls_sum_old = 0;
				calls_sum = 0;
				operator_mccmnc = 0;
				mute_state_old = 0;
				mute_state = 0;
				ret = QLRIL_SetMute(&handle, mute_state);
			}

			ret = QAUDIO_StopVoiceCall(&sndHandle);
			if (ret < 0)
				QLOGE("QAUDIO_StopVoiceCall error:%d\n", ret);

			break;
		}
		default:
			break;
		}
		QLOGD("Call state[%s] Exit\n", strCallState[new_state]);

		usleep(50000);
	}

	if (sndHandle != 0) {
		ret = QAUDIO_Exit(&sndHandle);
		QLOGD("QAUDIO_Exit return:%d", ret);
	}

	QLOGD("sound_status_switch thread stoped");
}

void processUnsolInd_cb(int *handle, int slotId, int event, void *data, size_t size)
{
	int i = 0;
	int id = 0;
	int ret = 0;
	int same_flag = 0;
	int holding_state = 0;
	list_node_t *node = NULL;
	struct calls_state *cstate = NULL;
	void *resp = NULL;
	char dev[] = "/dev/smd7";

	RIL_Call *calls = NULL;
	ims_CallList *call_list = NULL;
	ims_CallList_Call **imsCalls = NULL;

	switch(event) {
	case RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED: //1001
		QLOGD("RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED message size: %d", size);

		//ret = QLRIL_GetCurrentCalls(handle, slotId, (void **)&calls);
		ret = QLRIL_GetCurrentCallsByAtCmd(dev, slotId, (void **)&calls);
		QLOGD("QLRIL_GetCurrentCallsByAtCmd items = %d", ret);

		if (ret >= 0) {
			if (calls == NULL)
				QLOGD("RIL_Call memory pointer is NULL");

			if (ret >= 20)
				QLOGD("Warning!!! Calls items is too many");

			pthread_mutex_lock(&s_stateMutex);
			calls_sum_old = calls_sum;
			calls_sum = ret;
			have_calls = 0;
			holding_state = 0;
			holding2active = 0;
			if (calls_sum == 0 || calls == NULL) {
				for (node = state_list; node != NULL; node = node->next) {
					cstate = (struct calls_state *)node->data;
					if (cstate == NULL)
						continue;

					free(cstate);
					node->data = NULL;
				}

				list_destroy(&state_list);
				state_list = NULL;

				cstate = (struct calls_state *)malloc(sizeof(struct calls_state));
				cstate->index = 0;
				cstate->state = 6;
				cstate->slotId = slotId;
				memset(cstate->number, 0, sizeof(cstate->number));
				state_list = list_create((void *)cstate);
			} else {
				for (i = 0; i < calls_sum && (calls != NULL); i++) {
					if (calls[i].isVoice == 0)
						continue;

					if (calls[i].state == 0)
						have_calls++;

					if (calls[i].state == 1) //holding
						holding_state++;
				}

				if (have_calls == 0 && holding_state > 1)
					holding2active = 1;

				if (calls_sum_old > calls_sum) {
					for (node = state_list; node != NULL; node = node->next) {
						cstate = (struct calls_state *)node->data;
						if (cstate == NULL)
							continue;

						free(cstate);
						node->data = NULL;
					}
					list_destroy(&state_list);
					state_list = NULL;
				}

				for (i = 0; i < calls_sum && (calls != NULL); i++) {
					if (calls[i].isVoice == 0)
						continue;

					if (calls[i].state == 0)
						have_calls = 1;

					same_flag = 0;
					for (node = state_list; node != NULL; node = node->next) {
						cstate = (struct calls_state *)node->data;
						if (cstate == NULL)
							continue;

						if ((calls[i].number != NULL) && (strncmp(cstate->number,
										calls[i].number, strlen(calls[i].number)) == 0)) {
							if ((cstate->index >= calls[i].index) && (cstate->state != calls[i].state)) {
								cstate->index = calls[i].index;
								cstate->state = calls[i].state;
								QLOGD("There is new call index[%d] state[%d]", cstate->index, cstate->state);
								continue;
							}
						}

						if (calls_sum == 1 && cstate->state == 6 && calls[i].state == 4) {
							same_flag = 1;
							cstate->index = calls[i].index;
							cstate->state = calls[i].state;

							if (calls[i].number != NULL) {
								snprintf(cstate->number, sizeof(cstate->number), "%s", calls[i].number);
								free(calls[i].number);
								calls[i].number = NULL;
							}

							if (calls[i].name) {
								free(calls[i].name);
								calls[i].name = NULL;
							}
						}

						if (cstate->index == calls[i].index &&
								cstate->state == calls[i].state) {
							if ((calls[i].number != NULL) && (strncmp(cstate->number,
											calls[i].number, strlen(calls[i].number)) == 0)) {
								same_flag = 1;
								break;
							}
						}
					}

					if (same_flag != 1) {
						same_flag = 0;
						cstate = (struct calls_state *)malloc(sizeof(struct calls_state));
						cstate->index = calls[i].index;
						cstate->state = calls[i].state;
						cstate->slotId = slotId;
						if (calls[i].number != NULL) {
							snprintf(cstate->number, sizeof(cstate->number), "%s", calls[i].number);
							free(calls[i].number);
							calls[i].number = NULL;
						}

                        QLOGD("There is new call index[%d] state[%d]",
                                cstate->index, cstate->state);

						if (calls[i].name) {
							free(calls[i].name);
							calls[i].name = NULL;
						}

						state_list = list_insert_beginning(state_list, (void *)cstate);
						if (!state_list)
							QLOGD("Error list_insert_beginning\n");
					}
				}
			}
			pthread_mutex_unlock(&s_stateMutex);

			if (calls) {
				free(calls);
				calls = NULL;
			}
		} else {
			QLOGD("QLRIL_GetCurrentCalls failed with return: %d", ret);
		}

		break;
	case IMS_UNSOL_RESPONSE_CALL_STATE_CHANGED: //1051
		//201 = ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
		QLOGD("IMS_UNSOL_RESPONSE_CALL_STATE_CHANGED message size: %d", size);
		ret = size;
		if (ret <= 0) {
			ret = QLRIL_IMSGetCurrentCalls(handle, slotId, &id, &resp);
		} else {
			ret = size;
			resp = data;
		}

		if (ret <= 0 || resp == NULL) {
			QLOGD("some/All of IMS call end");
		} else {
			call_list = (ims_CallList *)resp;
			imsCalls = (ims_CallList_Call **)call_list->callAttributes.arg;
		}

		pthread_mutex_lock(&s_stateMutex);
		calls_sum = 0;
		have_calls = 0;
		holding_state = 0;
		holding2active = 0;
		if (ret == 0 || resp == NULL || imsCalls == NULL) {
			for (node = state_list; node != NULL; node = node->next) {
				cstate = (struct calls_state *)node->data;
				if (cstate == NULL)
					continue;

				free(cstate);
				node->data = NULL;
			}

			list_destroy(&state_list);
			state_list = NULL;

			cstate = (struct calls_state *)malloc(sizeof(struct calls_state));
			cstate->index = 0;
			cstate->state = 6;
			cstate->slotId = slotId;
			memset(cstate->number, 0, sizeof(cstate->number));
			state_list = list_create((void *)cstate);
		} else {
			for (i = 0; imsCalls[i] != NULL; i++) {
				if (imsCalls[i]->has_isVoice && !imsCalls[i]->isVoice)
					continue;

				calls_sum++;

				if (imsCalls[i]->has_state) {
					if (imsCalls[i]->state == 0)
						have_calls++;
					if (imsCalls[i]->state == 1)//holding
						holding_state++;
				}

				same_flag = 0;
				for (node = state_list; node != NULL; node = node->next) {
					cstate = (struct calls_state *)node->data;
					if (cstate == NULL)
						continue;

					if ((imsCalls[i]->number.arg != NULL) && (strncmp(cstate->number,
									imsCalls[i]->number.arg, strlen(imsCalls[i]->number.arg)) == 0)) {
						if ((cstate->index >= imsCalls[i]->index) && (cstate->state != imsCalls[i]->state)) {
							cstate->index = imsCalls[i]->index;
							cstate->state = imsCalls[i]->state;
							same_flag = 1;
							QLOGD("There is new call index[%d] state[%d]", cstate->index, cstate->state);
							continue;
						}
					}

					if (cstate->index == imsCalls[i]->index &&
							cstate->state == imsCalls[i]->state) {
						if ((imsCalls[i]->number.arg != NULL) && (strncmp(cstate->number,
										imsCalls[i]->number.arg, strlen(imsCalls[i]->number.arg)) == 0)) {
							same_flag = 1;
							break;
						}
					}
				}

				if (same_flag != 1) {
					same_flag = 0;
					QLOGD("There is new call state");
					cstate = (struct calls_state *)malloc(sizeof(struct calls_state));
					cstate->index = imsCalls[i]->index;
					cstate->state = imsCalls[i]->state;
					cstate->slotId = slotId;
					if (imsCalls[i]->number.arg != NULL) {
						snprintf(cstate->number, sizeof(cstate->number), "%s",
								(char *)imsCalls[i]->number.arg);
					}
					QLOGD("There is new call index[%d] state[%d]",
							cstate->index, cstate->state);

					if (state_list != NULL) {
						state_list = list_insert_beginning(state_list, (void *)cstate);
						if (!state_list)
							QLOGD("Error list_insert_beginning");
					} else {
						state_list = list_create((void *)cstate);
					}
				}
			}

			if (have_calls == 0 && holding_state > 1)
				holding2active = 1;
		}
		pthread_mutex_unlock(&s_stateMutex);
		break;
	default:
		if (event == 1050)
			qlril_exit = 1;
		QLOGD("event = %d", event);
		break;
	}
}

static int soundmgr_config_sync(void)
{
	int ret = 0;
	char *str = NULL;
	char buf[20] = {0};
	dictionary *ini = NULL;
	static struct stat state_new, state_old;

	if (access(SOUNDMGR_CONFIG, F_OK) != 0) {
		handsfree = 1;

		goto exit;
	}

	ret = stat(SOUNDMGR_CONFIG, &state_new);
	if (ret != 0 || state_new.st_mtime == state_old.st_mtime) {
		return 0;
	}

	state_old.st_mtime = state_new.st_mtime;

	ini = iniparser_load(SOUNDMGR_CONFIG);
	if (ini != NULL) {
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "%s", "general:on");
		soundmgr_start = iniparser_getboolean(ini, buf, 1);

		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "%s", "Voice:handsFree");
		handsfree = iniparser_getboolean(ini, buf, 1);

		iniparser_freedict(ini);//free ini
		ini = NULL;
	}

exit:
	if (sndHandle != 0) {
		ret = QAUDIO_SetHandsFree(&sndHandle, handsfree);
		if (ret < 0)
			QLOGW("QAUDIO_SetHandsFree error:%d", ret);
	}

	QLOGD("Sound Manager voice Hands-Free:%d", handsfree);

	return 0;
}

#define KEY_OTHER_EVENT_FILE_NAME	"/dev/input/event"
#define INPUT_EVENT_DIR_NAME		"/sys/class/input/event"

static int get_event_handle(char *name, int *handle)
{
	int i = 0;
	int ret = 0;
	char input_file[50];
	char input_name[50];
	FILE *tmpfp = NULL;

	if (name == NULL || strlen(name) <= 3) {
		 QLOGE("parameter name is invalid");
		 return -1;
	}

	if (handle == NULL) {
		 QLOGE("parameter handle is NULL");
		 return -2;
	}

	for (i = 0; i < 10; i++) {
		memset(input_file, 0, sizeof(input_file));
		memset(input_name, 0, sizeof(input_name));
		sprintf(input_file, "%s%d/device/name", INPUT_EVENT_DIR_NAME, i);

		if (access(input_file, F_OK))
			continue;

		tmpfp = fopen(input_file, "r");
		if (tmpfp == NULL) {
			QLOGE("fopen input file");
			continue;
		}

		if (fgets(input_name, sizeof(input_name) - 1, tmpfp) != NULL) {
			fclose(tmpfp);
			tmpfp = NULL;

			if (strstr(input_name, name) != NULL) {
				ret = 1;
				break;
			}
		}
	}

	if (i < 10 && ret == 1) {
		memset(input_file, 0, sizeof(input_file));
		sprintf(input_file, "%s%d", KEY_OTHER_EVENT_FILE_NAME, i);
		ret = open(input_file, O_RDONLY);
		if (ret < 0) {
			QLOGE("open event failed");
		} else {
			QLOGI("open %s: success", input_file);
			*handle = ret;
			ret = fcntl(ret, F_SETFL, O_NONBLOCK);
			if (ret < 0)
				QLOGE("fcntl failed:%d", ret);

			ret = 0;
		}
	}

	return ret;
}

static int adjust_volume(int level)
{
	int ret = 0;
	int state = 0;

	//ret = QAUDIO_StopMusic(&sndHandle);
	state = QAUDIO_IsInVoiceCall(&sndHandle);
	if (state < 0)
		return -1;

	pthread_mutex_lock(&s_audioMutex);
	if (sndHandle != 0) {
		if (state == 1) {
			ret = QAUDIO_AddVoiceVolume(&sndHandle, level);
			if (ret < 0)
				QLOGW("QAUDIO_AddVoiceVolume error:%d", ret);
		} else {
			ret = QAUDIO_AddOutputVolume(&sndHandle, level);
			if (ret < 0)
				QLOGW("QAUDIO_AddOutputVolume error:%d", ret);
		}
	}
	pthread_mutex_unlock(&s_audioMutex);

	if (0 && state == 0) {
		pthread_mutex_lock(&s_audioMutex);
		ret = play_wav_file("Altair.wav", MODE_NONBLOCK, 0);
		pthread_mutex_unlock(&s_audioMutex);
	}

	return 0;
}

static void *process_vol_key_and_jack(void *args)
{
	int i = 0;
	int ret = 0;
	int tmpfd = 0;
	int maxfd = 0;
	int volup_fd = 0;
	int voldown_fd = 0;
	int headsetJack_fd = 0;
	int buttonJack_fd = 0;
	struct timeval tv;
	struct input_event value[4];

	fd_set read_fds;

	for (;;) {
		FD_ZERO(&read_fds);

		if (volup_fd == 0) {
			ret = get_event_handle("gpio-keys", &volup_fd);
			if (ret == 0)
				QLOGI("Get volume up event success");
			else
				volup_fd = 0;
		}

		if (voldown_fd == 0) {
			ret = get_event_handle("qpnp_pon", &voldown_fd);
			if (ret == 0)
				QLOGI("Get volume down event success");
			else
				voldown_fd = 0;
		}

		if (headsetJack_fd == 0) {
			ret = get_event_handle("Headset Jack", &headsetJack_fd);
			if (ret == 0)
				QLOGI("Get Headset Jack event success");
			else
				headsetJack_fd = 0;
		}

		if (buttonJack_fd == 0) {
			ret = get_event_handle("Button Jack", &buttonJack_fd);
			if (ret == 0)
				QLOGI("Get Headset Jack event success");
			else
				buttonJack_fd = 0;
		}

		if (volup_fd != 0) {
			FD_SET(volup_fd, &read_fds);
			if (maxfd < volup_fd)
				maxfd = volup_fd;
		}

		if (voldown_fd != 0) {
			FD_SET(voldown_fd, &read_fds);
			if (maxfd < voldown_fd)
				maxfd = voldown_fd;
		}

		if (headsetJack_fd != 0) {
			FD_SET(headsetJack_fd, &read_fds);
			if (maxfd < headsetJack_fd)
				maxfd = headsetJack_fd;
		}

		if (buttonJack_fd != 0) {
			FD_SET(buttonJack_fd, &read_fds);
			if (maxfd < buttonJack_fd)
				maxfd = buttonJack_fd;
		}

		if (maxfd <= 0) {
			sleep(1);
			continue;
		}

		tv.tv_sec = 30;
		tv.tv_usec = 0;
		ret = select(maxfd + 1, &read_fds, NULL, NULL, &tv);
		if (ret <= 0) {
			if (ret == 0)
				continue;

			QLOGE("select failed:%d", ret);

			if (volup_fd != 0) {
				close(volup_fd);
				volup_fd = 0;
			}

			if (voldown_fd != 0) {
				close(voldown_fd);
				voldown_fd = 0;
			}

			if (headsetJack_fd != 0) {
				close(headsetJack_fd);
				headsetJack_fd = 0;
			}

			if (buttonJack_fd != 0) {
				close(buttonJack_fd);
				buttonJack_fd = 0;
			}

			continue;
		}

		if (FD_ISSET(volup_fd, &read_fds)) {
			ret = read(volup_fd, value, sizeof(value));
			ret /= sizeof(struct input_event);
			for (i = 0; i < ret; i++) {
				QLOGD("num:%d code:%d value:%d", i, value[i].code, value[i].value);
				if (value[i].code == 115 && value[i].value == 1) {
					ret = adjust_volume(10);
				}

				//for msm8909
				if (value[i].code == 114 && value[i].value == 1) {
					ret = adjust_volume(-10);
				}
			}
		}

		if (FD_ISSET(voldown_fd, &read_fds)) {
			ret = read(voldown_fd, value, sizeof(value));
			ret /= sizeof(struct input_event);
			for (i = 0; i < ret; i++) {
				QLOGD("num:%d code:%d value:%d", i, value[i].code, value[i].value);
				if (value[i].code == 114 && value[i].value == 1) {
					ret = adjust_volume(-10);
				}
			}
		}

		if (FD_ISSET(headsetJack_fd, &read_fds)) {
			ret = read(headsetJack_fd, value, sizeof(value));
			ret /= sizeof(struct input_event);
			for (i = 0; i < ret; i++) {
				QLOGD("num:%d code:%d value:%d", i, value[i].code, value[i].value);
			}
		}

		if (FD_ISSET(buttonJack_fd, &read_fds)) {
			ret = read(buttonJack_fd, value, sizeof(value));
			ret /= sizeof(struct input_event);
			for (i = 0; i < ret; i++) {
				QLOGD("num:%d code:%d value:%d", i, value[i].code, value[i].value);
			}
		}
	}

	return;
}

int main (int argc, char **argv)
{
	int i = 0;
	int ret = 0;
	int callback = 0;
	pthread_t tid = 0;
	pthread_t eid = 0;
	struct sound_strategy ss;

	QLOGD("Sound Manager starting...");
	soundmgr_config_sync();

	while (soundmgr_start == 0) {
		sleep(3);
	}

	for (;;) {
		if (sndHandle == 0) {
			ret = QAUDIO_Init(&sndHandle);
			if (ret < 0) {
				QLOGE ("QAUDIO_Init ret = %d", ret);
				sndHandle = 0;
				sleep(1);
				continue;
			} else {
				ret = play_wav_file("telephone_ring.wav", MODE_BLOCK, 0);
				if (ret < 0)
					QLOGE("play_wav_file error:%d", ret);
				ret = prepare_voice_call();
				if (ret < 0)
					QLOGD("prepare_voice_call return:%d", ret);
			}
		}

		if (eid == 0 && sndHandle != 0) {
			QLOGD("Initiate key event success");
			ret = pthread_create(&eid, NULL, process_vol_key_and_jack,
					(void *)NULL);
			if (ret != 0) {
				QLOGE("Error: pthread_create");
				eid = 0;
			} else {
				QLOGD("Create key event thread success");
			}
		}

		if ((tid == 0) && (sndHandle != 0)) {
			QLOGD("Initiate audio success");
			ret = pthread_create(&tid, NULL, sound_status_switch,
					(void *)NULL);
			if (ret != 0) {
				QLOGE("Error: pthread_create");
				tid = 0;
			} else {
				QLOGD("Create sound switch thread success");
				break;
			}
		}

		sleep(1);
	}

	for (;;) {
		if (handle == 0 || qlril_exit == 1) {
			callback = 0;
			qlril_exit = 0;
			if (handle != 0) {
				ret = QLRIL_Exit(&handle);
				QLOGE("QLRIL_Exit return:%d", ret);
				handle = 0;
			}

			ret = QLMISC_IsRunning("qlrild");
			if (ret > 0) {
				QLOGD("Sound Manager started");
				ret = QLRIL_Init(&handle);
				if (ret != 0 || handle == 0) {
					QLOGE("QLRIL_Init failure return:%d", ret);
					handle = 0;
				}
			}
		}

		if ((callback == 0) && (tid != 0) && (handle != 0)) {
			ret = QLRIL_AddUnsolEventsListener(&handle, processUnsolInd_cb, NULL);
			if (ret != 0) {
				ret = QLRIL_Exit(&handle);
				if (ret != 0)
					QLOGE("QLRIL_Exit failed return:%d", ret);
				handle = 0;
				callback = 0;
			} else {
				callback = 1;
				mute_state_old = 0;
				ret = QLRIL_SetMute(&handle, mute_state_old);

				ret = QLRIL_IMSServerEnable(&handle, 1); //start IMS server
				ret = QLRIL_RegisterUnsolEvents(&handle, 1001, 1001);
				ret += QLRIL_RegisterUnsolEvents(&handle, 1051, 1051);
				if (ret != 0) {
					ret = QLRIL_Exit(&handle);
					if (ret != 0)
						QLOGE("QLRIL_Exit failed return: %d", ret);
					handle = 0;
					callback = 0;
				} else
					QLOGD("QLRIL_RegisterUnsolEvents success");
			}

			QLRIL_SetDefaultTimeout(&handle, 5);
		}

		do {
			sleep(3);
			ret = soundmgr_config_sync();
			if (soundmgr_start == 0) {
				if (handle != 0) {
					ret = QLRIL_Exit(&handle);
					QLOGE("QLRIL_Exit return:%d", ret);
					handle = 0;
				}
			}
		} while(soundmgr_start == 0);
	}

	return 0;
} /* ----- End of main() ----- */
