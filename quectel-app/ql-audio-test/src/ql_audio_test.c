/**
 * Copyright:  (C) 2021 Quectel
 *             All rights reserved.
 *  Filename:  ql_audio_test.c
 *   Version:  1.0.0
 *    Author:  Peeta Chen <peeta.chen@quectel.com>
 * ChangeLog:  1, Release initial version on 20210301
 */

#include <stdio.h>
#include <stdlib.h>

#include <qlaudio_api.h>

int main (int argc, char **argv)
{
	char *file = "/data/qlaudiotest";
	struct Audio_Record record;
	struct Audio_Playback audio;

	record.device = DefaultInputDevice;
	record.channel = DefaultChannel;
	record.rate = DefaultRecordRate;

	printf("QL_Audio_Record start\n");
	QL_Audio_Record(record, file);
	printf("QL_Audio_Record end\n");

	audio.rate = DefualtPlayRate;
	audio.device = DefaultOutputDevice;
	audio.channel = DefaultChannel;
	audio.volume = 10;
	audio.filetype = Wav;
	//audio.filetype = Mp3;

	printf("QL_Audio_Playback start\n");
	QL_Audio_Playback(audio, file);
	printf("QL_Audio_Playback end\n");

	return 0;
} /* ----- End of main() ----- */
