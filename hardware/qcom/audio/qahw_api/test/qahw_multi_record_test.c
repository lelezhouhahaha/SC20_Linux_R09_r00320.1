/*
 * Copyright (c) 2016-2018, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 *
 * Copyright (C) 2015 The Android Open Source Project *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Test app to record multiple audio sessions at the HAL layer */
#define LOG_NDEBUG 0
#define LOG_NDDEBUG 0
#define  LOG_TAG "javed"

#include <utils/Log.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <cutils/list.h>
#include <signal.h>
#include <errno.h>
#include "qahw_api.h"
#include "qahw_defs.h"
#include "qlaudio_api.h"

#define nullptr NULL
#define LATENCY_NODE "/sys/kernel/debug/audio_in_latency_measurement_node"
#define LATENCY_NODE_INIT_STR "1"
#define MAX_RECORD_SESSIONS 6

static bool kpi_mode;
static FILE * log_file = NULL;
volatile bool stop_record = false;

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define FORMAT_PCM 1

struct wav_header {
	uint32_t riff_id;
	uint32_t riff_sz;
	uint32_t riff_fmt;
	uint32_t fmt_id;
	uint32_t fmt_sz;
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;       /* sample_rate * num_channels * bps / 8 */
	uint16_t block_align;     /* num_channels * bps / 8 */
	uint16_t bits_per_sample;
	uint32_t data_id;
	uint32_t data_sz;
};

struct audio_config_params {
	qahw_module_handle_t *qahw_mod_handle;
	audio_io_handle_t handle;
	audio_devices_t input_device;
	audio_input_flags_t flags;
	audio_config_t config;
	audio_source_t source;
	int channels;
	double record_delay;
	double record_length;
	char profile[50];
	char kvpairs[256];
	bool timestamp_mode;
	char timestamp_file_in[256];
	//char *filename;
	char filename[256];
};

struct timed_params {
	struct listnode list;
	char param[256];
	int param_delay;
};

static pthread_mutex_t glock;
static volatile int tests_running;
static volatile int tests_completed;

int sourcetrack_done = 0;
static pthread_mutex_t sourcetrack_lock;
struct qahw_sound_focus_param sound_focus_data;
void *context = NULL;
qahw_module_handle_t *qahw_mod_handle = NULL;

static bool request_wake_lock(bool wakelock_acquireds, bool enable)
{
	int system_ret;

	if (enable) {
		if (!wakelock_acquireds) {
			system_ret = system("echo audio_services > /sys/power/wake_lock");
			if (system_ret < 0) {
				fprintf(stderr, "%s.Failed to acquire audio_service lock\n", __func__);
				ALOGD( "%s.Failed to acquire audio_service lock\n", __func__);
			} else {
				wakelock_acquireds = true;
				ALOGD( "%s.Success to acquire audio_service lock\n", __func__);
			}
		} else
			ALOGD( "%s.Lock is already acquired\n", __func__);
	}

	if (!enable) {
		if (wakelock_acquireds) {
			system_ret = system("echo audio_services > /sys/power/wake_unlock");
			if (system_ret < 0) {
				fprintf(stderr, "%s.Failed to release audio_service lock\n", __func__);
				ALOGD( "%s.Failed to release audio_service lock\n", __func__);
			} else {
				wakelock_acquireds = false;
				ALOGD( "%s.Success to release audio_service lock\n", __func__);
			}
		} else
			ALOGD( "%s.No Lock is acquired to release\n", __func__);
	}
	return wakelock_acquireds;
}

static void stop_signal_handler(int signal __unused)
{
	stop_record = true;
}

void read_soundfocus_param(void)
{
	uint16_t start_angle[4] = {0};
	uint8_t enable_sector[4] = {0};
	uint16_t gain_step;

	//printf("Enter soundfocusparams startangle\n");
	scanf("%hd %hd %hd %hd",&start_angle[0], &start_angle[1],
			&start_angle[2], &start_angle[3]);
	memcpy(&sound_focus_data.start_angle, start_angle, sizeof(start_angle));

	//printf("Enter soundfocusparams enablesector\n");
	scanf("%hhd %hhd %hhd %hhd",&enable_sector[0], &enable_sector[1],
			&enable_sector[2], &enable_sector[3]);
	memcpy(&sound_focus_data.enable, enable_sector, sizeof(enable_sector));

	//printf("Enter soundfocusparams gainstep\n");
	scanf("%hd",&gain_step);
	memcpy(&sound_focus_data.gain_step, &gain_step, sizeof(gain_step));
}

void sourcetrack_signal_handler(int signal __unused)
{
	/* Function to read keyboard interupt to enable user to set parameters
	   for sourcetracking usecase Dynamically */

	pthread_mutex_lock(&sourcetrack_lock);
	read_soundfocus_param();
	pthread_mutex_unlock(&sourcetrack_lock);
}

void *read_sourcetrack_data(void* data)
{
	int idx =0, status = 0,count = 0, sect = 0;
	qahw_param_id param;
	qahw_param_payload payload;
	qahw_module_handle_t *qawh_module_handle = NULL;

	qawh_module_handle = (qahw_module_handle_t *)data;

	while (1) {
		sleep(1);
		if (sourcetrack_done == 1)
			return NULL;

		pthread_mutex_lock(&sourcetrack_lock);
		payload = (qahw_param_payload)sound_focus_data;
		param = QAHW_PARAM_SOUND_FOCUS;
		status = qahw_set_param_data(qawh_module_handle, param, &payload);
		if (status != 0)
			ALOGD( "Error.Failed Set SoundFocus Params\n");

		ALOGD( "\nGet SoundFocus Params from app");
		status = qahw_get_param_data(qawh_module_handle, param, &payload);
		if (status < 0) {
			ALOGD( "Failed to get sound focus params\n");
		} else {
			memcpy (&sound_focus_data, &payload.sf_params,
					sizeof(struct qahw_sound_focus_param));
			for (idx = 0; idx < 4; idx++){
				ALOGD( "\nstart_angle[%d]=%d",idx,
						payload.sf_params.start_angle[idx]);
				ALOGD( " enable[%d]=%d",idx,
						payload.sf_params.enable[idx]);
				ALOGD( " gain=%d\n",payload.sf_params.gain_step);
			}
		}
		param = QAHW_PARAM_SOURCE_TRACK;
		status = qahw_get_param_data(qawh_module_handle, param, &payload);
		if (status < 0) {
			fprintf (log_file, "Failed to get source tracking params\n");
		} else {
			for (idx = 0; idx < 4; idx++){
				ALOGD( "vad[%d]=%d ",idx, payload.st_params.vad[idx]);
				if (idx < 3)
					ALOGD( "doa_noise[%d]=%d \n",
							idx, payload.st_params.doa_noise[idx]);
			}
			ALOGD( "doa_speech=%d\n",payload.st_params.doa_speech);
			ALOGD( "polar_activity:");
			for (sect = 0; sect < 4; sect++ ){
				ALOGD( "\nSector No-%d:\n",sect + 1);
				idx = sound_focus_data.start_angle[sect];
				count = sound_focus_data.start_angle[(sect + 1)%4] -
					sound_focus_data.start_angle[sect];
				if (count <0)
					count = count + 360;
				do {
					ALOGD( "%d,",payload.st_params.polar_activity[idx%360]);
					count--;
					idx++;
				} while (count);
				ALOGD( "\n");
			}
		}
		pthread_mutex_unlock(&sourcetrack_lock);
	}
}

void test_begin()
{
	pthread_mutex_lock(&glock);
	tests_running++;
	pthread_mutex_unlock(&glock);
}

void test_end()
{
	pthread_mutex_lock(&glock);
	tests_running--;
	tests_completed++;
	pthread_mutex_unlock(&glock);
}

void *start_input(void *thread_param)
{
	int rc = 0, ret = 0, count = 0;
	FILE *fdLatencyNode = nullptr;
	struct timespec tsColdI = { 0, 0 };
	struct timespec tsColdF = { 0, 0 };
	struct timespec tsCont = { 0, 0 };
	uint64_t tCold, tCont, tsec, tusec;
	char latencyBuf[200] = {0};
	time_t start_time = time(0);
	double time_elapsed = 0;
	ssize_t bytes_read = -1;
	char param[100] = "audio_stream_profile=";


	qahw_in_buffer_t in_buf;
	static int64_t timestamp = 1;

	struct audio_config_params* params = (struct audio_config_params*) thread_param;
	qahw_module_handle_t *qahw_mod_handle = params->qahw_mod_handle;

	char *filename = params->filename;
	int data_sz = 0, name_len = strlen(filename);
	/* convert/check params before use */
	switch(params->channels) {
		case 1:
			params->config.channel_mask = AUDIO_CHANNEL_IN_MONO;
			break;
		case 2:
			params->config.channel_mask = AUDIO_CHANNEL_IN_STEREO;
			break;
		case 4:
			params->config.channel_mask = AUDIO_CHANNEL_INDEX_MASK_4;
			break;
		case 8:
			params->config.channel_mask = AUDIO_CHANNEL_INDEX_MASK_8;
			break;
		default:
			ALOGD( "ERROR :::: channle count %d not supported, handle(%d)", params->channels, params->handle);
			if (log_file != stdout)
				fprintf(stdout, "ERROR :::: channle count %d not supported, handle(%d)", params->channels, params->handle);
			pthread_exit(0);
	}

	/* Turn BT_SCO on if bt_sco recording */
	if(audio_is_bluetooth_sco_device(params->input_device)) {
		int ret = -1;
		const char * bt_sco_on = "BT_SCO=on";
		ret = qahw_set_parameters(qahw_mod_handle, bt_sco_on);
		ALOGD( " param %s set to hal with return value %d\n", bt_sco_on, ret);
	}

	/* setup debug node if in kpi mode */
	if (kpi_mode) {
		fdLatencyNode = fopen(LATENCY_NODE,"r+");
		if (fdLatencyNode) {
			ret = fwrite(LATENCY_NODE_INIT_STR, sizeof(LATENCY_NODE_INIT_STR), 1, fdLatencyNode);
			if (ret < 1)
				ALOGD( "error(%d) writing to debug node!, handle(%d)", ret, params->handle);
			fflush(fdLatencyNode);
		} else {
			ALOGD( "debug node(%s) open failed!, handle(%d)", LATENCY_NODE, params->handle);
			if (log_file != stdout)
				fprintf(stdout, "debug node(%s) open failed!, handle(%d)", LATENCY_NODE, params->handle);
			pthread_exit(0);
		}
	}

	test_begin();
	/* Open audio input stream */
	qahw_stream_handle_t* in_handle = NULL;

	rc = qahw_open_input_stream(qahw_mod_handle,
			params->handle, params->input_device,
			&params->config, &in_handle,
			params->flags, "input_stream",
			params->source);
	if (rc) {
		ALOGD( "ERROR :::: Could not open input stream, handle(%d)\n", params->handle);
		if (log_file != stdout)
			fprintf(stdout, "ERROR :::: Could not open input stream, handle(%d)\n", params->handle);
		test_end();
		pthread_exit(0);
	}

	/* Get buffer size to get upper bound on data to read from the HAL */
	size_t buffer_size = qahw_in_get_buffer_size(in_handle);
	char *buffer = (char *)calloc(1, buffer_size);
	size_t written_size;
	if (buffer == NULL) {
		ALOGD( "calloc failed!!, handle(%d)\n", params->handle);
		if (log_file != stdout)
			fprintf(stdout, "calloc failed!!, handle(%d)\n", params->handle);
		test_end();
		pthread_exit(0);
	}

	ALOGD( " input opened, buffer  %p, size %zu, handle(%d)", buffer, buffer_size, params->handle);
	/* set profile for the recording session */
	//strlcat(param, params->profile, sizeof(param));
	snprintf(param, sizeof(param), "%s%s", param, params->profile);
	qahw_in_set_parameters(in_handle, param);

	/* Caution: Below ADL log shouldnt be altered without notifying automation APT since it used for
	 * automation testing
	 */
	ALOGD( "\n ADL: Please speak into the microphone for %lf seconds, handle(%d)\n", params->record_length, params->handle);
	if (log_file != stdout)
		fprintf(stdout, "\n ADL: Please speak into the microphone for %lf seconds, handle(%d)\n", params->record_length, params->handle);

	ret = audio_is_linear_pcm(params->config.format);
	if (ret) {
		snprintf(filename + name_len, sizeof(params->filename), "%s", ".wav");
	} else {
		//snprintf(filename + name_len, sizeof(".raw"), ".raw");
		snprintf(filename + name_len, sizeof(params->filename), "%s", ".raw");
	}

	FILE *fd = fopen(filename,"w");
	if (fd == NULL) {
		ALOGD( "File open failed \n");
		if (log_file != stdout)
			fprintf(stdout, "File open failed \n");
		free(buffer);
		test_end();
		pthread_exit(0);
	}

	FILE *fd_in_ts = NULL;
	if (params->timestamp_mode) {
		if (*(params->timestamp_file_in))
			fd_in_ts = fopen(params->timestamp_file_in, "w+");
		if (fd_in_ts == NULL) {
			ALOGD( "playback timestamps file open failed \n");
			if (log_file != stdout)
				fprintf(stdout, "playback timestamps file open failed \n");
			test_end();
			pthread_exit(0);
		}
	}
	int bps = 16;

	switch(params->config.format) {
		case AUDIO_FORMAT_PCM_24_BIT_PACKED:
			bps = 24;
			break;
		case AUDIO_FORMAT_PCM_8_24_BIT:
		case AUDIO_FORMAT_PCM_32_BIT:
			bps = 32;
			break;
		case AUDIO_FORMAT_PCM_16_BIT:
		default:
			bps = 16;
	}

	struct wav_header hdr;
	hdr.riff_id = ID_RIFF;
	hdr.riff_sz = 0;
	hdr.riff_fmt = ID_WAVE;
	hdr.fmt_id = ID_FMT;
	hdr.fmt_sz = 16;
	hdr.audio_format = FORMAT_PCM;
	hdr.num_channels = params->channels;
	hdr.sample_rate = params->config.sample_rate;
	hdr.byte_rate = hdr.sample_rate * hdr.num_channels * (bps/8);
	hdr.block_align = hdr.num_channels * (bps/8);
	hdr.bits_per_sample = bps;
	hdr.data_id = ID_DATA;
	hdr.data_sz = 0;
	if (audio_is_linear_pcm(params->config.format))
		fwrite(&hdr, 1, sizeof(hdr), fd);

	memset(&in_buf,0, sizeof(qahw_in_buffer_t));
	start_time = time(0);
	while(true && !stop_record) {
		if(time_elapsed < params->record_delay) {
			usleep(1000000*(params->record_delay - time_elapsed));
			time_elapsed = difftime(time(0), start_time);
			continue;
		} else if (time_elapsed > params->record_delay + params->record_length) {
			ALOGD( "\n Test for session with handle(%d) completed.\n", params->handle);
			if (log_file != stdout)
				fprintf(stdout, "\n Test for session with handle(%d) completed.\n", params->handle);
			break;
		}

		if (kpi_mode && count == 0) {
			ret = clock_gettime(CLOCK_REALTIME, &tsColdI);
			if (ret)
				ALOGD( "error(%d) getting current time before first read!, handle(%d)", ret, params->handle);
		}

		in_buf.buffer = buffer;
		in_buf.bytes = buffer_size;
		if (params->timestamp_mode)
			in_buf.timestamp = &timestamp;
		bytes_read = qahw_in_read(in_handle, &in_buf);

		if (params->timestamp_mode)
			fprintf(fd_in_ts, "timestamp:%lld\n", timestamp);
		if (kpi_mode) {
			if (count == 0) {
				ret = clock_gettime(CLOCK_REALTIME, &tsColdF);
				if (ret)
					ALOGD( "error(%d) getting current time after first read!, handle(%d)", ret, params->handle);
			} else if (count == 8) {
				/* 8th read done time is captured in kernel which would have trigger 9th read in DSP
				 * 9th read is received by usersace at this time
				 */
				ret = clock_gettime(CLOCK_REALTIME, &tsCont);
				if (ret)
					ALOGD( "error(%d) getting current time after 8th read!, handle(%d)", ret, params->handle);
			}
			count++;
		}

		time_elapsed = difftime(time(0), start_time);
		written_size = fwrite(in_buf.buffer, 1, bytes_read, fd);
		if (written_size < bytes_read) {
			ALOGE("Error in fwrite(%d)=%s",ferror(fd), strerror(ferror(fd)));
			break;
		}
		data_sz += bytes_read;
	}
	if  ((params->timestamp_mode) && fd_in_ts) {
		fclose(fd_in_ts);
		fd_in_ts = NULL;
	}

	/*Stopping sourcetracking thread*/
	sourcetrack_done = 1;

	if (audio_is_linear_pcm(params->config.format)) {
		/* update lengths in header */
		hdr.data_sz = data_sz;
		hdr.riff_sz = data_sz + 44 - 8;
		fseek(fd, 0, SEEK_SET);
		fwrite(&hdr, 1, sizeof(hdr), fd);
	}
	free(buffer);
	fclose(fd);
	fd = NULL;

	/* capture latency kpis if required */
	if (kpi_mode) {
		tCold = tsColdF.tv_sec*1000 - tsColdI.tv_sec*1000 +
			tsColdF.tv_nsec/1000000 - tsColdI.tv_nsec/1000000;

		if (fdLatencyNode) {
			fread((void *) latencyBuf, 100, 1, fdLatencyNode);
			fclose(fdLatencyNode);
			fdLatencyNode = NULL;
		}
		sscanf(latencyBuf, " %llu,%llu", &tsec, &tusec);
		tCont = ((uint64_t)tsCont.tv_sec)*1000 - tsec*1000 + ((uint64_t)tsCont.tv_nsec)/1000000 - tusec/1000;
		if (log_file != stdout) {
			fprintf(stdout, "\n cold latency %llums, continuous latency %llums, handle(%d)\n", tCold, tCont, params->handle);
			fprintf(stdout, " **Note: please add DSP Pipe/PP latency numbers to this, for final latency values\n");
		}
		ALOGD( "\n values from debug node %s, handle(%d)\n", latencyBuf, params->handle);
		ALOGD( "\n cold latency %llums, continuous latency %llums, handle(%d)\n", tCold, tCont, params->handle);
		ALOGD( " **Note: please add DSP Pipe/PP latency numbers to this, for final latency values\n");
	}

	ALOGD( " closing input, handle(%d)", params->handle);

	/* Close input stream and device. */
	rc = qahw_in_standby(in_handle);
	if (rc) {
		ALOGD( "out standby failed %d, handle(%d)\n",rc, params->handle);
		if (log_file != stdout)
			fprintf(stdout, "out standby failed %d, handle(%d)\n",rc, params->handle);
	}

	rc = qahw_close_input_stream(in_handle);
	if (rc) {
		ALOGD( "could not close input stream %d, handle(%d)\n",rc, params->handle);
		if (log_file != stdout)
			fprintf(stdout, "could not close input stream %d, handle(%d)\n",rc, params->handle);
	}

	/* Print instructions to access the file.
	 * Caution: Below ADL log shouldnt be altered without notifying automation APT since it used for
	 * automation testing
	 */
	ALOGD( "\n\n ADL: The audio recording has been saved to %s. Please use adb pull to get "
			"the file and play it using audacity. The audio data has the "
			"following characteristics:\n Sample rate: %i\n Format: %d\n "
			"Num channels: %i, handle(%d)\n\n",
			filename, params->config.sample_rate, params->config.format, params->channels, params->handle);
	if (log_file != stdout)
		fprintf(stdout, "\n\n ADL: The audio recording has been saved to %s. Please use adb pull to get "
				"the file and play it using audacity. The audio data has the "
				"following characteristics:\n Sample rate: %i\n Format: %d\n "
				"Num channels: %i, handle(%d)\n\n",
				filename, params->config.sample_rate, params->config.format, params->channels, params->handle);

	test_end();
	return NULL;
}

int read_config_params_from_user(struct audio_config_params *thread_param) {
	//printf("Enter input device (4->built-in mic, 16->wired_headset .. etc)\n");
	scanf(" %d", &thread_param->input_device);
	thread_param->input_device |= AUDIO_DEVICE_BIT_IN;

	//printf("Enter the format (1 ->16 bit pcm recording, 6 -> 24 bit packed pcm recording)\n");
	scanf(" %d", &thread_param->config.format);

	//printf("Enter input flag to be used (0 -> none, 1 -> fast ...)\n");
	scanf(" %d", &thread_param->flags);

	//printf("Enter the sample rate (48000, 16000 etc)\n");
	scanf(" %d", &thread_param->config.sample_rate);

	//printf("Enter the channels (1 -mono, 2 -stereo and 4 -quad channels)\n");
	scanf(" %d", &thread_param->channels);

	//printf("Enter profile (none, record_fluence, record_mec, record_unprocessed etc)\n");
	scanf(" %s", thread_param->profile);

	//printf("Enter the audio source ( ref: system/media/audio/include/system/audio.h)\n");
	scanf(" %d", &thread_param->source);

	//printf("Enter the record duration in seconds\n");
	scanf(" %lf", &thread_param->record_length);

	//printf("Enter the start delay for the session in seconds\n");
	scanf(" %lf", &thread_param->record_delay);

	return 0;
}

void fill_default_params(struct audio_config_params *thread_param, int rec_session) {
	memset(thread_param,0, sizeof(struct audio_config_params));

	thread_param->input_device = AUDIO_DEVICE_IN_BUILTIN_MIC;
	thread_param->config.format = AUDIO_FORMAT_PCM_16_BIT;
	thread_param->channels = 2;
	thread_param->flags = (audio_input_flags_t)AUDIO_INPUT_FLAG_NONE;
	thread_param->config.sample_rate = 48000;
	thread_param->source = 1;
	thread_param->record_length = 60 /*sec*/;
	thread_param->record_delay = 0 /*sec*/;
	thread_param->timestamp_mode = false;

	thread_param->handle = 0x99A - rec_session;
}


static void qti_audio_server_death_notify_cb(void *ctxt __unused) {
	ALOGD( "qas died\n");
	stop_record = true;
}

bool source_tracking = false;
pthread_t sourcetrack_thread;
bool wakelock_acquired_s = false;
pthread_t tid[MAX_RECORD_SESSIONS];
int thread_actives[MAX_RECORD_SESSIONS] = { 0 };
int audio_record(int argc, char* argv[],char *file) {
	int max_recordings_requested = 0, status = 0;


	const  char *mod_name = "audio.primary";
	struct audio_config_params params[MAX_RECORD_SESSIONS];
	bool interactive_mode = false;
	struct listnode param_list;
	char log_filename[256] = "stdout";

	int i;
	const char *recording_session[MAX_RECORD_SESSIONS] = {"first", "second", "third", "fourth", "fifth", "sixth"};

	log_file = stdout;
	list_init(&param_list);
	fill_default_params(&params[0], 1);
	struct option long_options[] = {
		/* These options set a flag. */
		{"device",          required_argument,    0, 'd'},
		{"format",          required_argument,    0, 'f'},
		{"flags",           required_argument,    0, 'F'},
		{"sample-rate",     required_argument,    0, 'r'},
		{"channels",        required_argument,    0, 'c'},
		{"source",          required_argument,    0, 's'},
		{"profile",         required_argument,    0, 'p'},
		{"recording-time",  required_argument,    0, 't'},
		{"recording-delay", required_argument,    0, 'D'},
		{"log-file",        required_argument,    0, 'l'},
		{"timestamp-file",  required_argument,    0, 'm'},
		{"kpi-mode",        no_argument,          0, 'K'},
		{"interactive",     no_argument,          0, 'i'},
		{"source-tracking", no_argument,          0, 'S'},
		{"kvpairs",         required_argument,    0, 'k'},
		{"help",            no_argument,          0, 'h'},
		{0, 0, 0, 0}
	};

	int opt = 0;
	int option_index = 0;

	memset(params[0].filename, 0, sizeof(params[0].filename));
	snprintf(params[0].filename, sizeof(params[0].filename), "%s", file);
	while ((opt = getopt_long(argc,
					argv,
					"-d:f:F:r:c:s:p:t:D:l:m:k:KiSh",
					long_options,
					&option_index)) != -1) {
		ALOGD( "for argument %c, value is %s\n", opt, optarg);
		switch (opt) {
			case 'd':
				if(memcmp(optarg,"0",sizeof(optarg)) ==0)
				{
					params[0].input_device= AUDIO_DEVICE_IN_BUILTIN_MIC;
				}
				else
				{
					params[0].input_device = atoll(optarg);
				}
				break;
			case 'f':
				params[0].config.format = atoll(optarg);
				break;
			case 'F':
				params[0].flags = atoll(optarg);
				break;
			case 'r':
				if(memcmp(optarg,"0",sizeof(optarg)) ==0)
				{
					params[0].config.sample_rate= 48000;
				}
				else
				{
					params[0].config.sample_rate = atoi(optarg);
				}
				break;
			case 'c':
				if(memcmp(optarg,"0",sizeof(optarg)) ==0)
				{
					params[0].channels= 2;
				}
				else
				{
					params[0].channels = atoi(optarg);
				}
				break;
			case 's':
				params[0].source = atoi(optarg);
				break;
			case 'p':
				snprintf(params[0].profile, sizeof(params[0].profile), "%s", optarg);
				break;
			case 't':
				params[0].record_length = atoi(optarg);
				break;
			case 'D':
				params[0].record_delay = atoi(optarg);
				break;
			case 'l':
				snprintf(log_filename, sizeof(log_filename), "%s", optarg);
				break;
			case 'm':
				params[0].timestamp_mode = true;
				snprintf(params[0].timestamp_file_in, sizeof(params[0].timestamp_file_in), "%s", optarg);
				break;
			case 'K':
				kpi_mode = true;
				break;
			case 'i':
				interactive_mode = true;
				break;
			case 'S':
				source_tracking = true;
				break;
			case 'k':
				snprintf(params[0].kvpairs, sizeof(params[0].kvpairs), "%s", optarg);
				break;
			case 'h':
				return 0;
				break;
		}
	}
	ALOGD( "registering qas callback");
	qahw_register_qas_death_notify_cb((audio_error_callback)qti_audio_server_death_notify_cb, context);

	wakelock_acquired_s = request_wake_lock(wakelock_acquired_s, true);
	qahw_mod_handle = qahw_load_module(mod_name);
	if(qahw_mod_handle == NULL) {
		ALOGD( " qahw_load_module failed");
		return -1;
	}
	ALOGD( " Starting audio hal multi recording test. \n");
	if (interactive_mode) {
		//printf("Enter logfile path (stdout or 1 for console out)\n");
		scanf(" %s", log_filename);
		//printf("Enter number of record sessions to be started \n");
		//printf("Maximum of %d record sessions are allowed", MAX_RECORD_SESSIONS);
		scanf(" %d", &max_recordings_requested);
		if (max_recordings_requested > MAX_RECORD_SESSIONS) {
			ALOGD( " INVALID input -- Max record sessions supported is %d -exit \n",
					MAX_RECORD_SESSIONS);
			if (log_file != stdout)
				fprintf(stdout, " INVALID input -- Max record sessions supported is %d -exit \n",
						MAX_RECORD_SESSIONS);
			return -1;
		}
	} else {
		max_recordings_requested = 1;
	}
	if (strcasecmp(log_filename, "stdout") && strcasecmp(log_filename, "1")) {
		if ((log_file = fopen(log_filename,"wb"))== NULL) {
			fprintf(stderr, "Cannot open log file %s\n", log_filename);
			/* continue to log to std out */
			log_file = stdout;
		}
	}

	for (i = max_recordings_requested; i > 0;  i--) {
		if (interactive_mode) {
			//printf("Enter the config params for %s record session \n", recording_session[i - 1]);
			fill_default_params(&params[i - 1], i);
			read_config_params_from_user(&params[i - 1]);
		}
		params[i - 1].qahw_mod_handle = qahw_mod_handle;
		thread_actives[i - 1] = 1;
		//printf("\n");
	}

	if (interactive_mode && status == 0) {
		int option = 0;

		//printf("Source Tracking enabled( 1 - Enable 0 - Disable)\n");
		scanf(" %d", &option);
		source_tracking = option ? true : false;

		//printf("Measure latency KPI values( 1 - Enable 0 - Disable)\n");
		scanf(" %d", &option);
		kpi_mode = option ? true : false;

		while(true) {
			char ch = 'y';
			//printf("SetParam command required(y/n)\n");
			scanf(" %c", &ch);
			if (ch != 'y' && ch != 'Y')
				break;
			struct timed_params *param = (struct timed_params *)
				calloc(1, sizeof(struct timed_params));
			if (param == NULL) {
				ALOGD( " \n Failed to alloc memory for param, ignoring param conf\n\n");
				if (log_file != stdout)
					fprintf(stdout, " \n Failed to alloc memory for param, ignoring param conf\n\n");
				continue;
			}
			//printf("Enter param kv pair\n");
			scanf("%s", param->param);
			//printf("Enter param delay in sec(time after which param need to be set)\n");
			scanf("%d", &param->param_delay);

			list_add_tail(&param_list, &param->list);
		}
	}

	/* set global setparams entered by user.
	 * Also other global setparams can be concatenated if required.
	 */
	if (params[0].kvpairs[0] != 0) {
		size_t len;
		len = strcspn(params[0].kvpairs, ",");
		while (len < strlen(params[0].kvpairs)) {
			params[0].kvpairs[len] = ';';
			len = strcspn(params[0].kvpairs, ",");
		}
		//printf("param %s set to hal\n", params[0].kvpairs);
		qahw_set_parameters(qahw_mod_handle, params[0].kvpairs);
	}

	int ret = -1;

	if (source_tracking && max_recordings_requested) {
		if (pthread_mutex_init(&sourcetrack_lock, NULL) != 0) {
			//printf("mutex init failed\n");
			source_tracking = 0;
			goto sourcetrack_error;
		}
		if (signal(SIGQUIT, sourcetrack_signal_handler) == SIG_ERR)
			ALOGD( "Failed to register SIGQUIT:%d\n",errno);
		else
			ALOGD("NOTE::::To set sourcetracking params at runtime press 'Ctrl+\\' from keyboard\n");
		read_soundfocus_param();
		//printf("Create source tracking thread\n");
		ret = pthread_create(&sourcetrack_thread,
				NULL, read_sourcetrack_data,
				(void *)qahw_mod_handle);
		if (ret) {
			ALOGE("Failed to create source tracking thread");
			source_tracking = 0;
			pthread_mutex_destroy(&sourcetrack_lock);
			goto sourcetrack_error;
		}
	}

	/* Register the SIGINT to close the App properly */
	if (signal(SIGINT, stop_signal_handler) == SIG_ERR)
		ALOGD( "Failed to register SIGINT:%d\n", errno);

	/* Register the SIGTERM to close the App properly */
	if (signal(SIGTERM, stop_signal_handler) == SIG_ERR)
		ALOGD( "Failed to register SIGTERM:%d\n", errno);

	for (i = 0; i < MAX_RECORD_SESSIONS; i++) {
		if (thread_actives[i] == 1) {
			ALOGD( "\n Create %s record thread \n", recording_session[i]);
			ret = pthread_create(&tid[i], NULL, start_input, (void *)&params[i]);
			if (ret) {
				status = -1;
				ALOGD( " Failed to create %s record thread \n", recording_session[i]);
				if (log_file != stdout)
					fprintf(stdout, " Failed to create %s record thread \n", recording_session[i]);
				thread_actives[i] = 0;
			}
		}
	}

	ALOGD( " All threads started \n");
	if (log_file != stdout)
		fprintf(stdout, " All threads started \n");

	/* set params if queued */
	time_t start_time = time(0);
	while (!list_empty(&param_list)) {
		struct listnode *node, *tempnode;
		struct timed_params *param;
		time_t curr_time = time(0);
		list_for_each_safe(node, tempnode, &param_list) {
			param = node_to_item(node, struct timed_params, list);
			if (curr_time - start_time > param->param_delay ||
					(tests_completed > 0 && tests_running == 0)) {
				if (curr_time - start_time > param->param_delay) {
					ret = qahw_set_parameters(qahw_mod_handle, param->param);
					ALOGD( "param %s set to hal with return value %d\n", param->param, ret);
				}
				list_remove(&param->list);
				free(param);
			}
		}
		usleep(10000);
	}

	ALOGD("Waiting for threads exit");
	if (log_file != stdout)
		fprintf(stdout, " Waiting for threads exit");

	for (i = 0; i < MAX_RECORD_SESSIONS; i++) {
		if (thread_actives[i] == 1) {
			pthread_join(tid[i], NULL);
			ALOGD( " after %s record thread exit \n", recording_session[i]);
		}
	}

sourcetrack_error:
	optind = 0;
	//qahw_mod_handle = NULL;
	for (i = 0; i < MAX_RECORD_SESSIONS; i++) {
		if (thread_actives[i] == 1) {
			pthread_join(tid[i], NULL);
		}
	}
	if (source_tracking) {
		pthread_join(sourcetrack_thread,NULL);
		pthread_mutex_destroy(&sourcetrack_lock);
		//printf("after source tracking thread exit\n");
	}

	ret = qahw_unload_module(qahw_mod_handle);
	if (ret) {
		ALOGD( "could not unload hal %d \n",ret);
	}

	/* Caution: Below ADL log shouldnt be altered without notifying automation APT since it used
	 * for automation testing
	 */
	if (log_file != NULL) {
		ALOGD( "\n ADL: Done with hal record test \n");
		if (log_file != stdout) {
			fprintf(stdout, "\n ADL: Done with hal record test \n");
			fclose(log_file);
			log_file = NULL;
		}
	}
	wakelock_acquired_s = request_wake_lock(wakelock_acquired_s, false);
	return 0;
}

void QL_Record_Stop()
{
	int ret = 0;
	optind = 0;
	qahw_mod_handle = NULL;
	test_end();
	if (source_tracking) {
		pthread_join(sourcetrack_thread, NULL);
		pthread_mutex_destroy(&sourcetrack_lock);
		//printf("after source tracking thread exit\n");
	}
	ret = qahw_unload_module(qahw_mod_handle);
	if (ret) {
		ALOGD( "could not unload hal %d \n", ret);
	}

	/* Caution: Below ADL log shouldnt be altered without notifying automation APT since it used
	 * for automation testing
	 */
	if (log_file != NULL) {
		ALOGD( "\n ADL: Done with hal record test \n");
		if (log_file != stdout) {
			fprintf(stdout, "\n ADL: Done with hal record test \n");
			fclose(log_file);
			log_file = NULL;
		}
	}
	wakelock_acquired_s = request_wake_lock(wakelock_acquired_s, false);
}

void QL_Audio_Record(struct Audio_Record record,char *file)
{
	char channel[30] ={0};
	char device[30]  ={0};
	char rate[30]    ={0};
	sprintf(channel, "%d", record.channel);
	sprintf(device, "%d", record.device);
	sprintf(rate, "%d", record.rate);

	char *str[30]={NULL,"-r",rate,"-c",channel,"-d",device};

	audio_record(7,str,file);
}
