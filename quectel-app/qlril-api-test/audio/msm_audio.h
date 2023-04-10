/*
 * Copyright:  (C) 2020 quectel All rights reserved.
 *  Filename:  msm_audio.h
 *   Version:  1.0.0
 *    Author:  Peeta <peeta.chen@quectel.com>
 * ChangeLog:  1, Release initial version on 2020.07.28
 */

#ifdef __cplusplus
extern "C" {
#endif

int audio_play_wav(int *handle, char *wavfile);
int audio_play_stop(int *handle);

int audio_stream_enable(int *handle);
int audio_stream_disable(int *handle);

int voice_stream_enable(int *handle);
int voice_stream_disable(int *handle);

int snd_card_init(int *handle);
int snd_card_exit(int *handle);

#ifdef __cplusplus
}
#endif
