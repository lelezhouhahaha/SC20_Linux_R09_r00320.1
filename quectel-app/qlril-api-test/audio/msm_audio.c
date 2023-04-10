/*
 * Copyright:  (C) 2020 quectel All rights reserved.
 *  Filename:  alsa_intf.c
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

#ifdef QLRILD_DEBUG

#ifndef LOG_TAG
#define LOG_TAG "QLRILD"
#endif
#include <cutils/log.h>

#define QLOGD_IF	ALOGD_IF
#define QLOGW_IF	ALOGW_IF
#define QLOGE_IF	ALOGE_IF
#define QLOGD		ALOGD
#define QLOGV		ALOGV
#define QLOGI		ALOGI
#define QLOGW		ALOGW
#define QLOGE		ALOGE

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

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

static pthread_mutex_t audioMutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct SoundHandle {
	snd_use_case_mgr_t *uc_mgr;
	struct pcm *voice_tube;
	struct pcm *voice_play;
	struct pcm *audio_tube;
	struct pcm *audio_play;
	int play_loop;
	int voice_enable;
}SoundHandle_t;

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

static int msm_pcm_set_params(struct pcm *pcm, int out, int bits, int period)
{
	int err = 0;
	struct snd_pcm_hw_params *hwparams;
	struct snd_pcm_sw_params *swparams;

	if (pcm == NULL) {
		QLOGE("[%s] pmc is NULL", __func__);
		return -1;
	}

	/* Allocate the snd_pcm_hw_params_t structure on the stack. */
	//snd_pcm_hw_params_alloca(&hwparams);
	hwparams = (struct snd_pcm_hw_params *)malloc(sizeof(struct snd_pcm_hw_params));
	if (hwparams == NULL){
		QLOGE("[%s] HW params alloc error: %s", __func__, strerror(errno));
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
		QLOGE("[%s] Set HW parames error: %s", __func__, pcm_error(pcm));
		return -3;
	}

	pcm->buffer_size = pcm_buffer_size(hwparams);
	pcm->period_size = pcm_period_size(hwparams);
	pcm->period_cnt = pcm->buffer_size/pcm->period_size;

	QLOGD_IF(QLRILD_DEBUG, "period_size (%d)\n", pcm->period_size);
	QLOGD_IF(QLRILD_DEBUG, "buffer_size (%d)\n", pcm->buffer_size);
	QLOGD_IF(QLRILD_DEBUG, "period_cnt  (%d)\n", pcm->period_cnt);

	swparams = (struct snd_pcm_sw_params*)malloc(sizeof(struct snd_pcm_sw_params));
	if (swparams == NULL){
		QLOGE("[%s] SW params alloc error: %s", __func__, strerror(errno));
		return -ENOMEM;
	}

	swparams->tstamp_mode = SNDRV_PCM_TSTAMP_NONE;
	swparams->period_step = 1;

#if 0
	if (out) {
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
	swparams->avail_min = pcm->period_size/(pcm->channels * 2);
	swparams->xfer_align = pcm->period_size/(pcm->channels * 2);
	swparams->start_threshold = pcm->period_size/(pcm->channels * 2);
	swparams->stop_threshold = pcm->buffer_size;

	swparams->silence_size = 0;
	swparams->silence_threshold = 0;

	if ((err = param_set_sw_params(pcm, swparams)) != 0) {
		QLOGE("param_set_sw_params: Error setting SW params:%s", pcm_error(pcm));
		return -errno;
	}

	QLOGD_IF(QLRILD_DEBUG, "avail_min (%lu)\n", swparams->avail_min);
	QLOGD_IF(QLRILD_DEBUG, "start_threshold (%lu)\n", swparams->start_threshold);
	QLOGD_IF(QLRILD_DEBUG, "stop_threshold (%lu)\n", swparams->stop_threshold);
	QLOGD_IF(QLRILD_DEBUG, "xfer_align (%lu)\n", swparams->xfer_align);
	QLOGD_IF(QLRILD_DEBUG, "boundary (%lu)", swparams->boundary);

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
		QLOGE("[%s] handle is invalid", __func__);
		return -1;
	}

	p = (SoundHandle_t *)*handle;

	if (pcmdev == NULL || strlen(pcmdev) <= 0) {
		QLOGE("%s: pcmdev is invalid", __func__);
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

	if ((p->voice_play = pcm_open(flags, pcmdev)) == NULL) {//pcmdev="hw:0,34"
		QLOGE("[%s] Open %s error: %s", __func__, pcmdev, strerror(errno));
		return -3;
	}

	if (!pcm_ready(p->voice_play)) {
		QLOGE("[%s] handle ready error: %s", __func__, strerror(errno));
		err = -4;
		goto error2;
	}

	p->voice_play->rate = rate;
	p->voice_play->flags = flags;
	p->voice_play->format = SNDRV_PCM_FORMAT_S16_LE;
	p->voice_play->channels = channels;

	err = msm_pcm_set_params(p->voice_play, 1, 16, period);
	if (err < 0){
		QLOGE("msm_pcm_set_params() error: %d", err);
		err = -5;
		goto error2;
	}

	flags |= PCM_IN;//0x10000000
	if ((p->voice_tube = pcm_open(flags, pcmdev)) == NULL) {//pcmdev="hw:0,34"
		QLOGE("[%s] Open %s error: %s", __func__, pcmdev, strerror(errno));
		err = -6;
		goto error2;
	}

	if (!pcm_ready(p->voice_tube)) {
		QLOGE("[%s] voice tube ready error: %s", __func__, strerror(errno));
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
	pcm_close(p->voice_tube);
	p->voice_tube = NULL;

error2:
	pcm_close(p->voice_play);
	p->voice_play = NULL;

	return err;
}

static int audio_pcm_open(int *handle, int channels, int rate)
{
	int err;
	int period = 0;
	unsigned flags = PCM_NMMAP;

	SoundHandle_t *p = NULL;

	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("[%s] handle is invalid", __func__);
		return -1;
	}

	p = (SoundHandle_t *)*handle;

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

	if ((p->audio_play = pcm_open(flags, "hw:0,0")) == NULL) {//pcmdev="hw:0,0"
		QLOGE("[%s] Error pcm_open", __func__);
		return -2;
	}

	pthread_mutex_lock(&audioMutex);
	if (!pcm_ready(p->audio_play)) {
		QLOGE("[%s] handle ready error: %s", __func__, strerror(errno));
		err = -3;
		pthread_mutex_unlock(&audioMutex);
		goto error1;
	}

	p->audio_play->rate = rate;
	p->audio_play->flags = flags;
	p->audio_play->format = SNDRV_PCM_FORMAT_S16_LE;
	p->audio_play->channels = channels;

	err = msm_pcm_set_params(p->audio_play, channels, 16, period);
	if (err < 0){
		QLOGE("msm_pcm_set_params() error: %d", err);
		err = -4;
		pthread_mutex_unlock(&audioMutex);
		goto error1;
	}
	pthread_mutex_unlock(&audioMutex);

	return 0;

error1:
	pcm_close(p->audio_play);
	p->audio_play = NULL;

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
		QLOGE("[%s] handle is invalid", __func__);
		return -1;
	}

	p = (SoundHandle_t *)*handle;

	if (wavfile == NULL || strlen(wavfile) <= 0) {
		errno = EINVAL;
		QLOGE("[%s] wavfile is invalid", __func__);
		return -2;
	}

	fd = open(wavfile, O_RDONLY);
	if (fd <= 0) {
		QLOGE("[%s] open %s failed", __func__, wavfile);
		return -3;
	}

	err = read(fd, &wavHeader, sizeof(wavHeader));
	if (err != sizeof(wavHeader)) {
		QLOGE("[%s] read wave header failed", __func__);
		return -4;
	}

	if ((wavHeader.riff_id != ID_RIFF) ||
			(wavHeader.riff_fmt != ID_WAVE) ||
			(wavHeader.fmt_id != ID_FMT)) {
		errno = 201;
		QLOGE("[%s] Error not a riff or wave file", __func__);
		return -5;
	}

	/*if ((wavHeader.audio_format != 1) ||
			(wavHeader.fmt_sz != 16)) {
		errno = 202;
		QLOGE("[%s] Error format or fmt_sz is unknown", __func__);
		return -6;
	}*/

	if (wavHeader.bits_per_sample != 16) {
		errno = 203;
		QLOGE("[%s] Error bits per sample not supported", __func__);
		return -7;
	}

	if (p->audio_play == NULL) {
		err = audio_pcm_open(handle, wavHeader.num_channels, wavHeader.sample_rate);
		if (err < 0) {
			errno = 204;
			QLOGE("[%s] audio_pcm_open error: %d", __func__, err);
			return -8;
		}
	}

	if (p->audio_play && !p->audio_play->running &&
			pcm_prepare(p->audio_play) != 0) {
		QLOGE("[%s] pcm_prepare voice play error: %d", __func__, err);
		//return -9;
	}

	p->play_loop = 1;

	remainingData = wavHeader.data_sz;
	bufsize = p->audio_play->period_size;
	if (remainingData < bufsize)
		bufsize = remainingData;

	data = (uint8_t *)malloc(sizeof(uint8_t) * bufsize);
	if (data == NULL) {
		QLOGE("[%s] malloc() error: %s", __func__, strerror(errno));
		return -10;
	}

	ret = -14;
	for (;p->play_loop && p->audio_play != NULL;) {
		memset(data, 0, bufsize);

		err = read(fd, data, bufsize);
		if (err <= 0) {
			ret = -11;
			break;
		}

		pthread_mutex_lock(&audioMutex);
		if (p->play_loop == 0 || p->audio_play == NULL) {
			pthread_mutex_unlock(&audioMutex);
			ret = -12;
			break;
		}

		err = pcm_write(p->audio_play, data, bufsize);
		pthread_mutex_unlock(&audioMutex);
		if (err != 0) {
			ret = -13;
			break;
		}

		remainingData -= bufsize;
		if (remainingData <= 0) {
			printf ("fulinux I am here\n");
			ret = 0;
			break;
		}

		if (remainingData < bufsize)
			bufsize = remainingData;
	}

	free(data);

	return ret;
}

int audio_play_stop(int *handle)
{
	int err = 0;

	SoundHandle_t *p = NULL;

	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("[%s] handle is invalid", __func__);
		return -1;
	}

	p = (SoundHandle_t *)*handle;
	pthread_mutex_lock(&audioMutex);
	p->play_loop = 0;
	if (p->audio_play != NULL) {
		if (ioctl(p->audio_play->fd, SNDRV_PCM_IOCTL_DROP) < 0) {
			QLOGE("Drop voice tube failed:%s", pcm_error(p->audio_play));
			//return -2;
		}
	}
	pthread_mutex_unlock(&audioMutex);
#if 0
	if (p->audio_play) {
		pcm_close(p->audio_play);
		p->audio_play = NULL;
	}
#endif
	return 0;
}

int voice_call_start(int *handle)
{
	int err = 0;
	SoundHandle_t *p = NULL;

	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("[%s] handle is invalid", __func__);
		return -1;
	}

	p = (SoundHandle_t *)*handle;

	if (p->voice_tube == NULL || p->voice_play == NULL) {
		QLOGE("[%s] voice tube or play is invalid", __func__);
		return -2;
	}

	if (pcm_prepare(p->voice_play) != 0) {
		QLOGE("[%s] pcm_prepare voice play error: %d", __func__, err);
		return -3;
	}

	err = ioctl(p->voice_play->fd, SNDRV_PCM_IOCTL_START);
	if (err < 0) {
		QLOGE("[%s] Error ioctl err:%d", __func__, err);
		return -4;
	}

	if (pcm_prepare(p->voice_tube) != 0) {
		QLOGE("[%s] pcm_prepare voice tube error: %d", __func__, err);
		return -5;
	}

	err = ioctl(p->voice_tube->fd, SNDRV_PCM_IOCTL_START);
	if (err < 0) {
		QLOGE("[%s] Error ioctl err:%d", __func__, err);
		return -6;
	}

	return 0;
}

int voice_call_stop(int *handle)
{
	int err = 0;
	SoundHandle_t *p = NULL;

	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("[%s] handle is invalid", __func__);
		return -1;
	}

	p = (SoundHandle_t *)*handle;

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

	return 0;
}

int voice_stream_enable(int *handle)
{
	int err = 0;
	SoundHandle_t *p = NULL;

	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("[%s] handle is invalid", __func__);
		return -1;
	}

	p = (SoundHandle_t *)*handle;

	err = snd_use_case_set(p->uc_mgr, "_verb", "Voice Call");
	if (err < 0) {
		QLOGE("snd_use_case_set voice call error: %s", strerror(errno));
		return -2;
	}

	//err = snd_use_case_set(uc_mgr, "_enadev", "Speaker");
	err = snd_use_case_set(p->uc_mgr, "_enadev", "Headphones");
	if (err < 0) {
		QLOGE("snd_use_case_set headphones error: %s", strerror(errno));
		return -3;
	}

	err = snd_use_case_set(p->uc_mgr, "_enadev", "Headset");
	if (err < 0) {
		QLOGE("snd_use_case_set headset error: %s", strerror(errno));
		return -4;
	}

	err = voice_pcm_open(handle, "hw:0,34", 1, 44100);
	if (err < 0) {
		QLOGE("[%s] voice_pcm_open error: %d", __func__, err);
		return -5;
	}

	err = voice_call_start(handle);
	if (err < 0) {
		QLOGE("[%s] voice_call_start error: %d", __func__, err);
		return -6;
	}

	p->voice_enable = 1;

	return 0;
}

int voice_stream_disable(int *handle)
{
	int err = 0;
	SoundHandle_t *p = NULL;

	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("[%s] handle is invalid", __func__);
		return -1;
	}

	p = (SoundHandle_t *)*handle;

	if (p->voice_enable == 0) {
		QLOGW("[%s] voice stream should enable firstly", __func__);
		return 0;
	}

	err = voice_call_stop(handle);
	if (err < 0) {
		QLOGE("[%s] voice_call_stop error: %d", __func__, err);
		return -2;
	}

	err = snd_use_case_set(p->uc_mgr, "_disdev", "Headphones");
	if (err < 0) {
		QLOGE("[%s] Disable Headphones error: %s", __func__, strerror(errno));
		return -3;
	}

	//err = snd_use_case_set(uc_mgr, "_enadev", "Speaker");
	err = snd_use_case_set(p->uc_mgr, "_disdev", "Headset");
	if (err < 0) {
		QLOGE("[%s] Disable headset error: %s", __func__, strerror(errno));
		return -4;
	}

	p->voice_enable = 0;

	return 0;
}

int audio_stream_enable(int *handle)
{
	int err = 0;
	SoundHandle_t *p = NULL;

	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("[%s] handle is invalid", __func__);
		return -1;
	}

	p = (SoundHandle_t *)*handle;

	err = snd_use_case_set(p->uc_mgr, "_verb", "HiFi");
	if (err < 0) {
		QLOGE("[%s] Use HiFi error: %s", __func__, strerror(errno));
		return -2;
	}

	//err = snd_use_case_set(uc_mgr, "_enadev", "Speaker");
	err = snd_use_case_set(p->uc_mgr, "_enadev", "Speaker");
	if (err < 0) {
		QLOGE("[%s] Enable Speaker error: %s", __func__, strerror(errno));
		return -3;
	}

	return 0;
}

int audio_stream_disable(int *handle)
{
	int err = 0;
	SoundHandle_t *p = NULL;

	printf ("fulinux audio_stream_disable 1\n");
	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("[%s] handle is invalid", __func__);
		return -1;
	}

	p = (SoundHandle_t *)*handle;

	p->play_loop = 0;

	pthread_mutex_lock(&audioMutex);
	if (p->audio_play) {
		pcm_close(p->audio_play);
		p->audio_play = NULL;
	}
	pthread_mutex_unlock(&audioMutex);

	printf ("fulinux audio_stream_disable 2 err: %d\n", err);
	err = snd_use_case_set(p->uc_mgr, "_verb", SND_USE_CASE_VERB_INACTIVE);
	printf ("fulinux audio_stream_disable 3 err: %d\n", err);
	if (err < 0) {
		QLOGE("[%s] Disable headset error: %s", __func__, strerror(errno));
		return -3;
	}

	printf ("fulinux audio_stream_disable 4\n");
	err = snd_use_case_set(p->uc_mgr, "_disdev", "Speaker");
	printf ("fulinux audio_stream_disable 5 err: %d\n", err);
	if (err < 0) {
		QLOGE("[%s] Disable Speaker error: %s", __func__, strerror(errno));
		return -2;
	}

	return 0;
}

int snd_card_init(int *handle)
{
	int err = 0;

	SoundHandle_t *p = (SoundHandle_t *)malloc(sizeof(SoundHandle_t));
	if (p == NULL)
		return -1;

	err = snd_use_case_mgr_open(&p->uc_mgr, "snd_soc_msm_9x07_Tomtom_I2S");
	if (err != 0) {
		QLOGE("snd_use_case_mgr_open error: %s", strerror(errno));
		return -2;
	}

	*handle = (int)p;
	QLOGD_IF(QLRILD_DEBUG, "snd_use_case_mgr_open success");

	return 0;
}

int snd_card_exit(int *handle)
{
	int err = 0;
	SoundHandle_t *p = NULL;

	if (handle == NULL || *handle == 0) {
		errno = EBADF;
		QLOGE("[%s] handle is invalid", __func__);
		return -1;
	}

	p = (SoundHandle_t *)*handle;

	err = snd_use_case_mgr_close(p->uc_mgr);
	if (err != 0) {
		QLOGE("snd_use_case_mgr_close error: %s", strerror(errno));
		return -2;
	}

	if (p->voice_tube > 0) {
		pcm_close(p->voice_tube);
		p->voice_tube = 0;
	}

	if (p->voice_play > 0) {
		pcm_close(p->voice_play);
		p->voice_play = 0;
	}

	pthread_mutex_lock(&audioMutex);
	if (p->audio_play) {
		pcm_close(p->audio_play);
		p->audio_play = NULL;
	}
	pthread_mutex_unlock(&audioMutex);

	free(p);
	p = NULL;
	*handle = 0;

	return 0;
}
