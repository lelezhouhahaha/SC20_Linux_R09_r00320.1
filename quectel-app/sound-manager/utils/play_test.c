/**
 *   Copyright:  (C) 2021 Quectel All rights reserved.
 *    Filename:  play_test.c
 *     Version:  1.0.0
 *      Author:  Peeta Chen <peeta.chen@quectel.com>
 *   ChangeLog:  1, Release initial version on "20210121"
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "msm_audio.h"

int main (int argc, char **argv)
{
	int ret = 0;
	int sndHandle = 0;

	ret = snd_card_init(&sndHandle);
	if (ret < 0) {
		printf ("snd_card_init failed:%d\n", ret);
		return -1;
	}

	//ret = audio_stream_enable(&sndHandle, "Headphones");
	ret = audio_stream_enable(&sndHandle, "Speaker");
	if (ret < 0) {
		printf ("audio_stream_enable failed:%d\n", ret);
		return -2;
	}

	sleep(1);

	ret = audio_play_wav(&sndHandle, "/etc/mmi/qualsound.wav");
	if (ret < 0) {
		printf ("audio_play_wav failed:%d\n", ret);
		return -3;
	}

	//ret = audio_stream_disable(&sndHandle, "Headphones");
	ret = audio_stream_disable(&sndHandle, "Speaker");
	if (ret < 0) {
		printf ("audio_stream_disable failed:%d\n", ret);
		return -4;
	}

	ret = snd_card_exit(&sndHandle);
	if (ret < 0) {
		printf ("snd_card_exit failed:%d\n", ret);
		return -5;
	}

	return 0;
} /* ----- End of main() ----- */

