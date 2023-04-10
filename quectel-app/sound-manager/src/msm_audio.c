/*
 * Copyright:  (C) 2020 quectel All rights reserved.
 *  Filename:  msm_audio.c
 *   Version:  1.0.0
 *    Author:  Peeta <peeta.chen@quectel.com>
 * ChangeLog:  1, Release initial version on "2020.07.28"
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

#include <sound/asound.h>
#include <alsa-intf-msm8909/alsa_ucm.h>
#include <alsa-intf-msm8909/alsa_audio.h>

#include "msm_audio.h"

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define CTLDEVICE	"/dev/snd/controlC0"

static pthread_mutex_t audioMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t voiceMutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct SoundHandle {
	snd_use_case_mgr_t *uc_mgr;
	struct pcm *voice_tube;
	struct pcm *voice_play;
	struct pcm *audio_tube;
	struct pcm *audio_play;
	int play_loop;
	int voice_enable;
	int uc_mgr_set;
} SoundHandle_t;

struct wav_header {
	uint32_t riff_id;
	uint32_t riff_sz;
	uint32_t riff_fmt;
	uint32_t fmt_id;
	uint32_t fmt_sz;
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;       /*  sample_rate * num_channels * bps / 8 */
	uint16_t block_align;     /*  num_channels * bps / 8 */
	uint16_t bits_per_sample;
	uint32_t data_id;
	uint32_t data_sz;
};

static struct mixer_ctl *get_ctl(struct mixer *mixer, char *name)
{
	char *p = NULL;
	uint32_t idx = 0;

	if (isdigit(name[0]))
		return mixer_get_nth_control(mixer, atoi(name) - 1);

	p = strrchr(name, '#');
	if (p) {
		*p++ = 0;
		idx = atoi(p);
	}

	return mixer_get_control(mixer, name, idx);
}

int audio_get_volume(char *name, int *val)
{
	int ret = 0;
	unsigned value;
	struct mixer *mixer;
	struct mixer_ctl *ctl;

	if (name == NULL || strlen(name) <= 0) {
		QLOGE("name is invalid");
		return -1;
	}

	if (val == NULL) {
		QLOGE("val is NULL");
		return -2;
	}

	mixer = mixer_open(CTLDEVICE);
	if (!mixer) {
		ALOGE("mixer_open failed");
		return -3;
	}

	ctl = get_ctl(mixer, name);
	if (ctl == NULL) {
		QLOGE("Get mixer ctl failed");
		mixer_close(mixer);
		return -4;
	}

	mixer_ctl_get(ctl, &value);

	*val = value;

	mixer_close(mixer);

	return ret;
}

int audio_set_volume(char *name, int val)
{
	int ret = 0;
	char *volume = NULL;
	struct mixer *mixer;
	struct mixer_ctl *ctl;

	if (name == NULL || strlen(name) <= 0) {
		QLOGE("[%s] name is invalid", __func__);
		return -1;
	}

	mixer = mixer_open(CTLDEVICE);
	if (!mixer) {
		QLOGE("mixer_open failed");
		return -2;
	}

	ctl = get_ctl(mixer, name);
	if (ctl == NULL) {
		QLOGE("[%s] Get mixer ctl failed", __func__);
		mixer_close(mixer);
		return -3;
	}

	if (val < 0)
		val = 0;

	if (val > 124)
		val = 124;

	volume = (char *)malloc(sizeof(char) * 200);
	snprintf(volume, 200, "%d", val);

	mixer_ctl_set_value(ctl, 1, &volume);

	free(volume);
	volume = NULL;

	//ret = mixer_ctl_set(ctl, val);

	mixer_close(mixer);

	return ret;
}

static int msm_pcm_set_params(struct pcm *pcm, int out, int bits, int period)
{
	int err = 0;
	struct snd_pcm_hw_params *hwparams;
	struct snd_pcm_sw_params *swparams;

	if (pcm == NULL) {
		QLOGE("pcm is NULL");
		return -1;
	}

	/* Allocate the snd_pcm_hw_params_t structure on the stack. */
	//snd_pcm_hw_params_alloca(&hwparams);
	hwparams = (struct snd_pcm_hw_params *)malloc(sizeof(struct snd_pcm_hw_params));
	if (hwparams == NULL){
		QLOGE("HW params alloc error: %s", strerror(errno));
		return -2;
	}

	param_init(hwparams);
	param_set_mask(hwparams, SNDRV_PCM_HW_PARAM_ACCESS,
			(pcm->flags & PCM_MMAP)? SNDRV_PCM_ACCESS_MMAP_INTERLEAVED:SNDRV_PCM_ACCESS_RW_INTERLEAVED);
	param_set_mask(hwparams, SNDRV_PCM_HW_PARAM_FORMAT, pcm->format);
	param_set_mask(hwparams, SNDRV_PCM_HW_PARAM_SUBFORMAT, SNDRV_PCM_SUBFORMAT_STD);
	if (period > 0)
		param_set_min(hwparams, SNDRV_PCM_HW_PARAM_PERIOD_BYTES, period);
	else
		param_set_min(hwparams, SNDRV_PCM_HW_PARAM_PERIOD_TIME, 10);

	param_set_int(hwparams, SNDRV_PCM_HW_PARAM_SAMPLE_BITS, bits);
	param_set_int(hwparams, SNDRV_PCM_HW_PARAM_FRAME_BITS, pcm->channels * bits);
	param_set_int(hwparams, SNDRV_PCM_HW_PARAM_CHANNELS, pcm->channels);
	param_set_int(hwparams, SNDRV_PCM_HW_PARAM_RATE, pcm->rate);

	param_set_hw_refine(pcm, hwparams);

	if ((err = param_set_hw_params(pcm, hwparams)) != 0) {
		QLOGE("Set HW parames error: %s", pcm_error(pcm));
		return -3;
	}

	pcm->buffer_size = pcm_buffer_size(hwparams);
	pcm->period_size = pcm_period_size(hwparams);
	pcm->period_cnt = pcm->buffer_size/pcm->period_size;

	/*QLOGD_IF("period_size (%d)\n", pcm->period_size);
	QLOGD_IF("buffer_size (%d)\n", pcm->buffer_size);
	QLOGD_IF("period_cnt  (%d)\n", pcm->period_cnt);*/

	swparams = (struct snd_pcm_sw_params*)malloc(sizeof(struct snd_pcm_sw_params));
	if (swparams == NULL){
		QLOGE("SW params alloc error: %s", strerror(errno));
		return -ENOMEM;
	}

	swparams->tstamp_mode = SNDRV_PCM_TSTAMP_NONE;
	swparams->period_step = 1;

#if 1
	if (out) {
		swparams->avail_min = pcm->period_size/(pcm->channels * 2);
		swparams->xfer_align = pcm->period_size/(pcm->channels * 2);
		swparams->start_threshold = pcm->period_size/(pcm->channels * 2);
		swparams->stop_threshold = pcm->buffer_size;
	} else {
		if (pcm->flags & PCM_MONO) {
			swparams->avail_min = pcm->period_size/2;
			swparams->xfer_align = pcm->period_size/2;
		} else if (pcm->flags & PCM_QUAD) {
			swparams->avail_min = pcm->period_size/8;
			swparams->xfer_align = pcm->period_size/8;
		} else if (pcm->flags & PCM_5POINT1) {
			swparams->avail_min = pcm->period_size/12;
			swparams->xfer_align = pcm->period_size/12;
		} else {
			swparams->avail_min = pcm->period_size/4;
			swparams->xfer_align = pcm->period_size/4;
		}

		swparams->start_threshold = 1;
		swparams->stop_threshold = INT_MAX;
	}
#endif

	swparams->silence_size = 0;
	swparams->silence_threshold = 0;

	if ((err = param_set_sw_params(pcm, swparams)) != 0) {
		QLOGE("param_set_sw_params: Error setting SW params:%d", err);
		return -4;
	}

	/*QLOGD_IF("avail_min (%lu)\n", swparams->avail_min);
	QLOGD_IF("start_threshold (%lu)\n", swparams->start_threshold);
	QLOGD_IF("stop_threshold (%lu)\n", swparams->stop_threshold);
	QLOGD_IF("xfer_align (%lu)\n", swparams->xfer_align);
	QLOGD_IF("boundary (%lu)", swparams->boundary);*/

	return 0;
}

static int voice_pcm_open(int *handle, char *pcmdev, int channels, int rate)
{
	int err;
	int period = 0;
	unsigned flags = PCM_NMMAP;

	SoundHandle_t *p = NULL;

	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("handle is invalid");
		return -1;
	}

	p = (SoundHandle_t *)*handle;

	if (pcmdev == NULL || strlen(pcmdev) <= 0) {
		QLOGE("pcmdev is invalid");
		return -2;
	}

	flags |= PCM_OUT;//0x00000000

	if (channels == 1)
		flags |= PCM_MONO;
	else if (channels == 2)
		flags |= PCM_STEREO;
	else if (channels == 4)
		flags |= PCM_QUAD;
	else if (channels == 6)
		flags |= PCM_5POINT1;
	else if (channels == 8)
		flags |= PCM_7POINT1;
	else {
		channels = 2;
		flags |= PCM_STEREO;
	}

	pthread_mutex_lock(&voiceMutex);
	if (p->voice_play != NULL)
		pcm_close(p->voice_play);
	p->voice_play = pcm_open(flags, pcmdev); //pcmdev="hw:0,34"
	pthread_mutex_unlock(&voiceMutex);
	if (p->voice_play == NULL) {//pcmdev="hw:0,34"
		QLOGE("Open %s error: %s", pcmdev, strerror(errno));
		return -3;
	}

	if (!pcm_ready(p->voice_play)) {
		QLOGE("handle ready error: %s", strerror(errno));
		err = -4;
		goto error2;
	}

	p->voice_play->rate = rate;
	p->voice_play->flags = flags;
	p->voice_play->format = SNDRV_PCM_FORMAT_S16_LE;
	p->voice_play->channels = channels;

	err = msm_pcm_set_params(p->voice_play, 1, 16, period);
	if (err < 0) {
		QLOGE("msm_pcm_set_params() error: %d", err);
		err = -5;
		goto error2;
	}

	flags |= PCM_IN;//0x10000000
	pthread_mutex_lock(&voiceMutex);
	if (p->voice_tube != NULL)
		pcm_close(p->voice_tube);
	p->voice_tube = pcm_open(flags, pcmdev); //pcmdev="hw:0,34"
	pthread_mutex_unlock(&voiceMutex);
	if (p->voice_tube == NULL) {//pcmdev="hw:0,34"
		QLOGE("Open %s error: %s", pcmdev, strerror(errno));
		err = -6;
		goto error2;
	}

	if (!pcm_ready(p->voice_tube)) {
		QLOGE("voice tube ready error: %s", strerror(errno));
		err = -7;
		goto error1;
	}

	p->voice_tube->rate = rate;
	p->voice_tube->flags = flags;
	p->voice_tube->format = SNDRV_PCM_FORMAT_S16_LE;
	p->voice_tube->channels = channels;

	err = msm_pcm_set_params(p->voice_tube, 0, 16, period);
	if (err < 0){
		QLOGE("msm_pcm_set_params() error: %d", err);
		err = -5;
		goto error1;
	}

	return 0;

error1:
	pthread_mutex_lock(&voiceMutex);
    if (p->voice_tube) {
        pcm_close(p->voice_tube);
        p->voice_tube = NULL;
    }
	pthread_mutex_unlock(&voiceMutex);

error2:
	pthread_mutex_lock(&voiceMutex);
    if (p->voice_play) {
        pcm_close(p->voice_play);
        p->voice_play = NULL;
    }
	pthread_mutex_unlock(&voiceMutex);

	return err;
}

static int audio_pcm_open(int *handle, int channels, int rate)
{
	int err;
	int period = 0;
	unsigned flags = PCM_NMMAP;

	SoundHandle_t *p = NULL;

	QLOGD("Entry");
	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("handle is invalid");
		return -1;
	}

	p = (SoundHandle_t *)*handle;

	flags |= PCM_OUT;//0x00000000
	//flags |= DEBUG_ON;

	if (channels == 1)
		flags |= PCM_MONO;
	else if (channels == 2)
		flags |= PCM_STEREO;
	else if (channels == 4)
		flags |= PCM_QUAD;
	else if (channels == 6)
		flags |= PCM_5POINT1;
	else if (channels == 8)
		flags |= PCM_7POINT1;
	else {
		channels = 2;
		flags |= PCM_STEREO;
	}

	pthread_mutex_lock(&audioMutex);
	p->audio_play = pcm_open(flags, "hw:0,0");//pcmdev="hw:0,0"
	pthread_mutex_unlock(&audioMutex);
	if (p->audio_play == NULL) {//pcmdev="hw:0,0"
		QLOGE("Error pcm_open");
		return -2;
	}

	if (!pcm_ready(p->audio_play)) {
		QLOGE("handle ready error: %s", strerror(errno));
		err = -3;
		goto error1;
	}

	p->audio_play->rate = rate;
	p->audio_play->flags = flags;
	p->audio_play->format = SNDRV_PCM_FORMAT_S16_LE;
	p->audio_play->channels = channels;

	err = msm_pcm_set_params(p->audio_play, 1, 16, period);
	if (err < 0){
		QLOGE("msm_pcm_set_params() error: %d", err);
		err = -4;
		goto error1;
	}

	QLOGD("Exit");
	return 0;

error1:
	pthread_mutex_lock(&audioMutex);
    if (p->audio_play) {
        pcm_close(p->audio_play);
        p->audio_play = NULL;
    }
	pthread_mutex_unlock(&audioMutex);

	QLOGD("Exit error:%d", err);

	return err;
}

int audio_play_wav(int *handle, char *wavfile)
{
	int err = 0;
	int ret = 0;
	int fd = 0;
	int bufsize = 0;
	uint8_t *data = NULL;
	int remainingData = 0;
	SoundHandle_t *p = NULL;
	struct wav_header wavHeader;

	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("handle is invalid");
        ret = -1;
        goto error2;
	}

	p = (SoundHandle_t *)*handle;

	if (wavfile == NULL || strlen(wavfile) <= 0) {
		errno = EINVAL;
		QLOGE("wavfile is invalid");
        ret = -2;
        goto error2;
	}

	fd = open(wavfile, O_RDONLY);
	if (fd <= 0) {
		QLOGE("open %s failed", wavfile);
        ret = -3;
        goto error2;
	}

	err = read(fd, &wavHeader, sizeof(wavHeader));
	if (err != sizeof(wavHeader)) {
		QLOGE("read wave header failed");
		ret = -4;
        goto error1;
	}

	if ((wavHeader.riff_id != ID_RIFF) ||
			(wavHeader.riff_fmt != ID_WAVE) ||
			(wavHeader.fmt_id != ID_FMT)) {
		errno = 201;
		QLOGE("Error not a riff or wave file");
		ret = -5;
        goto error1;
	}

	/*if ((wavHeader.audio_format != 1) ||
			(wavHeader.fmt_sz != 16)) {
		errno = 202;
		QLOGE("Error format or fmt_sz is unknown");
		return -6;
	}*/

	if (wavHeader.bits_per_sample != 16) {
		errno = 203;
		QLOGE("Error bits per sample not supported");
		ret = -7;
        goto error1;
	}

	if (p->audio_play == NULL) {
		err = audio_pcm_open(handle, wavHeader.num_channels, wavHeader.sample_rate);
		if (err < 0) {
			errno = 204;
            p->audio_play = NULL;
			QLOGE("audio_pcm_open error: %d", err);
			ret = -8;
            goto error1;
		}
	}

	if (p->audio_play == NULL) {
		QLOGE("audio_play is NULL");
		ret = -9;
        goto error1;
	}

	if (pcm_prepare(p->audio_play) != 0) {
		ALOGE("[%s] pcm_prepare error", __func__);
	}

	remainingData = wavHeader.data_sz;
	bufsize = p->audio_play->period_size * 2;
	if (remainingData > 0 && remainingData < bufsize)
		bufsize = remainingData;

	data = (uint8_t *)malloc(sizeof(uint8_t) * bufsize);
	if (data == NULL) {
		QLOGE("malloc() error: %s", strerror(errno));
		ret = -10;
        goto error1;
	}

	ret = 0;
	p->play_loop = 1;
	for (;(p->play_loop == 1) && p->audio_play != NULL;) {
		memset(data, 0, bufsize);

		err = read(fd, data, bufsize);
		if (err < 0) {
			ret = -11;
			break;
		}

		if (err == 0) {
			ret = 0;
			break;
		}

		if (p->play_loop != 1 || p->audio_play == NULL) {
			ret = 0;
			p->play_loop = 2;
			break;
		}

		err = pcm_write(p->audio_play, data, bufsize);
		if (err != 0) {
		    QLOGE("pcm_write error: %d, bufsize = %d", err, bufsize);
			ret = -13;
			break;
		}

		if (remainingData > err)
			remainingData -= err;

		if (remainingData <= 0) {
			ret = 0;
			break;
		}

		if (remainingData < bufsize) {
			bufsize = remainingData;
			//QLOGD ("fulinux remainingData = %d\n", remainingData);
		}
	}

	if (data) {
		free(data);
		data = NULL;
	}

error1:
    close(fd);

	pthread_mutex_lock(&audioMutex);
	if (p->audio_play) {
		pcm_close(p->audio_play);
		p->audio_play = NULL;
	}
	pthread_mutex_unlock(&audioMutex);

error2:

	return ret;
}

int audio_play_stop(int *handle)
{
	int i = 0;
	int err = 0;

	SoundHandle_t *p = NULL;

	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("handle is invalid");
		return -1;
	}

	p = (SoundHandle_t *)*handle;
	if (p->play_loop == 1) {
		p->play_loop = 0;
		for (i = 0; i < 10; i++) {
			if (p->play_loop == 2)
				break;
			usleep(10000);
		}
	}
	p->play_loop = 0;

	if (p->audio_play != NULL) {
		/*if (ioctl(p->audio_play->fd, SNDRV_PCM_IOCTL_DROP) < 0) {
			QLOGE("Drop voice tube failed:%s", pcm_error(p->audio_play));
		}*/
	}

#if 0
	pthread_mutex_lock(&voiceMutex);
	if (p->audio_play) {
		pcm_close(p->audio_play);
		p->audio_play = NULL;
	}
	pthread_mutex_unlock(&voiceMutex);
#endif

	return 0;
}

int voice_call_start(int *handle)
{
	int err = 0;
	SoundHandle_t *p = NULL;

	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("handle is invalid");
		return -1;
	}

	p = (SoundHandle_t *)*handle;

	if (p->voice_tube == NULL || p->voice_play == NULL) {
		QLOGE("voice tube or play is invalid");
		return -2;
	}

	if (pcm_prepare(p->voice_play) != 0) {
		QLOGE("pcm_prepare voice play error: %d", err);
	}

	err = ioctl(p->voice_play->fd, SNDRV_PCM_IOCTL_START);
	if (err < 0) {
		QLOGE("Error ioctl err:%d", err);
		return -3;
	}

	if (pcm_prepare(p->voice_tube) != 0) {
		QLOGE("pcm_prepare voice play error: %d", err);
	}

	err = ioctl(p->voice_tube->fd, SNDRV_PCM_IOCTL_START);
	if (err < 0) {
		QLOGE("Error ioctl err:%d", err);
		return -4;
	}

	return 0;
}

int voice_call_stop(int *handle)
{
	int err = 0;
	SoundHandle_t *p = NULL;

	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("handle is invalid");
		return -1;
	}

	p = (SoundHandle_t *)*handle;

	pthread_mutex_lock(&voiceMutex);
	if (p->voice_tube != NULL) {
		if (ioctl(p->voice_tube->fd, SNDRV_PCM_IOCTL_DROP) < 0) {
			QLOGE("Drop voice tube failed:%s", pcm_error(p->voice_tube));
			//return -2;
		}

		pcm_close(p->voice_tube);
		p->voice_tube = NULL;
	}

	if (p->voice_play != NULL) {
		if (ioctl(p->voice_play->fd, SNDRV_PCM_IOCTL_DROP) < 0) {
			QLOGE("Drop voice play failed:%s", pcm_error(p->voice_play));
			//return -3;
		}

		pcm_close(p->voice_play);
		p->voice_play = NULL;
	}
	pthread_mutex_unlock(&voiceMutex);

	return 0;
}

int voice_stream_enable(int *handle, char *output, char *input)
{
	int err = 0;
	SoundHandle_t *p = NULL;

	QLOGD("Entry");
	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("handle is invalid");
		return -1;
	}

	if (output == NULL || input == NULL) {
		errno = EINVAL;
		QLOGE("output/input is invalid");
		return -2;
	}

	if (strlen(output) == 0 ||
			!(strcmp(output, "Speaker") == 0 ||
			 strcmp(output, "Headphones") == 0)) {
		errno = EINVAL;
		QLOGE("output is not Speaker or Headphones");
		return -2;
	}

	if (strlen(input) == 0 ||
			!(strcmp(input, "Headset") == 0 ||
			 strcmp(input, "Handset") == 0)) {
		errno = EINVAL;
		QLOGE("input is not Headset or Handset");
		return -2;
	}

	p = (SoundHandle_t *)*handle;

	if (p->uc_mgr == NULL) {
		QLOGE("uc_mgr is NULL");
		return -3;
	}

	if (p->voice_enable == 1) {
		QLOGE("voice call has enabled");
		return 0;
	}

	err = snd_use_case_set(p->uc_mgr, "_verb", "Voice Call");
	if (err < 0) {
		QLOGE("snd_use_case_set voice call err: %d", err);
		return -4;
	}

	//err = snd_use_case_set(uc_mgr, "_enadev", "Speaker");
	//err = snd_use_case_set(p->uc_mgr, "_enadev", "Headphones");
	err = snd_use_case_set(p->uc_mgr, "_enadev", output);
	if (err < 0) {
		QLOGE("snd_use_case_set %s err: %d", output);
		return -5;
	}

	//err = snd_use_case_set(p->uc_mgr, "_enadev", "Headset");
	err = snd_use_case_set(p->uc_mgr, "_enadev", input);
	if (err < 0) {
		QLOGE("snd_use_case_set %s err: %d", input, err);
		return -6;
	}

	err = voice_pcm_open(handle, "hw:0,34", 1, 44100);
	if (err < 0) {
		QLOGE("voice_pcm_open error: %d", err);
		return -7;
	}

	err = voice_call_start(handle);
	if (err < 0) {
		QLOGE("voice_call_start error: %d", err);
		//return -8;
	}

	p->voice_enable = 1;
	QLOGD("Exit");

	return 0;
}

int voice_stream_disable(int *handle, char *output, char *input)
{
	int err = 0;
	SoundHandle_t *p = NULL;

	QLOGD("Entry");
	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("handle is invalid");
		return -1;
	}

	if (output == NULL || input == NULL) {
		errno = EINVAL;
		QLOGE("output/input is invalid");
		return -2;
	}

	if (strlen(output) == 0 ||
			!(strcmp(output, "Speaker") == 0 ||
			 strcmp(output, "Headphones") == 0)) {
		errno = EINVAL;
		QLOGE("output is not Speaker or Headphones");
		return -3;
	}

	if (strlen(input) == 0 ||
			!(strcmp(input, "Headset") == 0 ||
			 strcmp(input, "Handset") == 0)) {
		errno = EINVAL;
		QLOGE("input is not Headset or Handset");
		return -4;
	}

	p = (SoundHandle_t *)*handle;

	if (p->voice_enable == 0) {
		QLOGW("voice stream should enable firstly");
		return 0;
	}

	err = voice_call_stop(handle);
	if (err < 0) {
		QLOGE("voice_call_stop error: %d", err);
		return -5;
	}

	if (p->uc_mgr == NULL) {
		QLOGE("uc_mgr is NULL");
		return -6;
	}

	err = snd_use_case_set(p->uc_mgr, "_verb", SND_USE_CASE_VERB_INACTIVE);
	if (err < 0) {
		QLOGE("Disable _verb error: %s", strerror(errno));
		return -7;
	}

	//err = snd_use_case_set(p->uc_mgr, "_disdev", "Headphones");
	err = snd_use_case_set(p->uc_mgr, "_disdev", output);
	if (err < 0) {
		QLOGE("Disable %s error: %s", output, strerror(errno));
		return -8;
	}

	//err = snd_use_case_set(p->uc_mgr, "_disdev", "Headset");
	err = snd_use_case_set(p->uc_mgr, "_disdev", input);
	if (err < 0) {
		QLOGE("Disable %s error: %s", input, strerror(errno));
		return -9;
	}

	p->voice_enable = 0;
	QLOGD("Exit");

	return 0;
}

int audio_stream_enable(int *handle, char *output)
{
	int err = 0;
	SoundHandle_t *p = NULL;

	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("handle is invalid");
		return -1;
	}

	if (output == NULL) {
		errno = EINVAL;
		QLOGE("output/input is invalid");
		return -2;
	}

	if (strlen(output) == 0 ||
			!(strcmp(output, "Speaker") == 0 ||
			 strcmp(output, "Headphones") == 0)) {
		errno = EINVAL;
		QLOGE("output is not Speaker or Headphones");
		return -3;
	}

	p = (SoundHandle_t *)*handle;
	if (p->uc_mgr == NULL) {
		QLOGE("uc_mgr is NULL");
		return -4;
	}

	if (p->uc_mgr_set == 1)
		return 0;

	//err = snd_use_case_set(p->uc_mgr, "_verb", "Play Music");
	err = snd_use_case_set(p->uc_mgr, "_verb", "HiFi");
	if (err < 0) {
		QLOGE("Use HiFi error: %s", strerror(errno));
		return -5;
	}

	//err = snd_use_case_set(uc_mgr, "_enadev", "Speaker");
	//err = snd_use_case_set(p->uc_mgr, "_enadev", "Headphones");
	err = snd_use_case_set(p->uc_mgr, "_enadev", output);
	if (err < 0) {
		QLOGE("Enable %s error: %s", output, strerror(errno));
		return -6;
	}

	p->uc_mgr_set = 1;

	return 0;
}

int audio_stream_disable(int *handle, char *output)
{
	int err = 0;
	SoundHandle_t *p = NULL;

	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("handle is invalid");
		return -1;
	}

	if (output == NULL) {
		errno = EINVAL;
		QLOGE("output/input is invalid");
		return -2;
	}

	if (strlen(output) == 0 ||
			!(strcmp(output, "Speaker") == 0 ||
			 strcmp(output, "Headphones") == 0)) {
		errno = EINVAL;
		QLOGE("output is not Speaker or Headphones");
		return -3;
	}

	p = (SoundHandle_t *)*handle;
    p->play_loop = 0;

	pthread_mutex_lock(&audioMutex);
	if (p->audio_play != NULL) {
		/*if (ioctl(p->audio_play->fd, SNDRV_PCM_IOCTL_DROP) < 0) {
			QLOGE("Drop voice tube failed:%s", pcm_error(p->audio_play));
		}*/

		pcm_close(p->audio_play);
		p->audio_play = NULL;
	}
	pthread_mutex_unlock(&audioMutex);

	/*
	err = snd_use_case_set(p->uc_mgr, "_verb", SND_USE_CASE_VERB_INACTIVE);
	if (err < 0) {
		QLOGE("Disable _verb error: %s", strerror(errno));
		return -4;
	}
	*/

	//err = snd_use_case_set(p->uc_mgr, "_disdev", "Speaker");
	//err = snd_use_case_set(p->uc_mgr, "_disdev", "Headphones");
	if (p->uc_mgr != NULL && p->uc_mgr_set == 1) {
		p->uc_mgr_set = 0;
		err = snd_use_case_set(p->uc_mgr, "_disdev", output);
		if (err < 0) {
			QLOGE("Disable %s error: %s", output, strerror(errno));
			return -5;
		}
	} else
		QLOGD("uc_mgr is none");

	return 0;
}

int snd_card_init(int *handle)
{
	int err = 0;

	SoundHandle_t *p = (SoundHandle_t *)malloc(sizeof(SoundHandle_t));
	if (p == NULL)
		return -1;

    memset(p, 0, sizeof(SoundHandle_t));

	err = snd_use_case_mgr_open(&p->uc_mgr, "snd_soc_msm_9x07_Tomtom_I2S");
	if (err != 0) {
		QLOGE("snd_use_case_mgr_open error: %s", strerror(errno));
		return -2;
	}

	*handle = (int)p;

	return 0;
}

int snd_card_exit(int *handle)
{
	int err = 0;
	SoundHandle_t *p = NULL;

	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("handle is invalid");
		return -1;
	}

	p = (SoundHandle_t *)*handle;

	if (p->uc_mgr != NULL) {
		err = snd_use_case_mgr_close(p->uc_mgr);
		if (err != 0) {
			QLOGE("snd_use_case_mgr_close error: %s", strerror(errno));
			return -2;
		}
	}

	pthread_mutex_lock(&voiceMutex);
	if (p->voice_tube != NULL) {
		pcm_close(p->voice_tube);
		p->voice_tube = NULL;
	}

	if (p->voice_play != NULL) {
		pcm_close(p->voice_play);
		p->voice_play = NULL;
	}
	pthread_mutex_unlock(&voiceMutex);

	pthread_mutex_lock(&audioMutex);
	if (p->audio_play) {
		pcm_close(p->audio_play);
		p->audio_play = NULL;
	}
	pthread_mutex_unlock(&audioMutex);

    if (p != NULL) {
        free(p);
        p = NULL;
    }
	*handle = 0;

	return 0;
}
