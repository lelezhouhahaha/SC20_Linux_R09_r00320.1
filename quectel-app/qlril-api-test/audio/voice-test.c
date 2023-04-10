/**
 * Copyright:  (C) 2020 Quectel
 *             All rights reserved.
 *  Filename:  voice-test.c
 *   Version:  1.0.0
 *    Author:  Peeta Chen <peeta.chen@quectel.com>
 * ChangeLog:  1, Release initial version on 20200729
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <msm_audio.h>

int main (int argc, char **argv)
{
	int ret = 0;
	int handle = 0;

	ret = snd_card_init(&handle);
	if (ret < 0) {
		printf ("snd_card_init ret = %d\n", ret);
		return -1;
	}

	ret = audio_stream_enable(&handle);
	if (ret < 0) {
		printf ("audio_stream_enable ret = %d\n", ret);
		goto error1;
	}

	ret = audio_play_wav(&handle, "/etc/mmi/qualsound.wav");
	if (ret < 0) {
		printf ("audio_play_wav ret = %d\n", ret);
		goto error1;
	}

	ret = audio_play_stop(&handle);
	if (ret < 0) {
		printf ("audio_play_stop ret = %d\n", ret);
		goto error1;
	}

	ret = audio_stream_disable(&handle);
	if (ret < 0) {
		printf ("audio_stream_disable ret = %d\n", ret);
		goto error1;
	}

	ret = voice_stream_enable(&handle);
	if (ret < 0) {
		printf ("voice_stream_enable ret = %d\n", ret);
		goto error1;
	}

	sleep(20);

	ret = voice_stream_disable(&handle);
	if (ret < 0) {
		printf ("voice_stream_disable ret = %d\n", ret);
		goto error1;
	}

error1:
	ret = snd_card_exit(&handle);
	if (ret < 0) {
		printf ("snd_card_exit ret = %d\n", ret);
	}

	return 0;
} /* ----- End of main() ----- */

