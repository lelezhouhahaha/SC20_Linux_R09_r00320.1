/*
** Copyright 2010, The Android Open-Source Project
** Copyright (c) 2011-2013, 2016 The Linux Foundation. All rights reserved.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include <limits.h>

#include <sound/asound.h>
#include <sound/compress_params.h>
#include <sound/compress_offload.h>

#include "alsa_audio.h"

#if 0
#ifndef ANDROID
#define strlcat g_strlcat
#define strlcpy g_strlcpy
#endif
#endif

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define FORMAT_PCM 1

#define COMPR_META_DATA_SIZE	64
static struct wav_header hdr;
static int fd;
static struct pcm *pcm;
static debug = 0;
static pcm_flag = 1;
static duration = 0;
static char *filename;
static char *data;
static int format = SNDRV_PCM_FORMAT_S16_LE;
static int period = 0;
static int piped = 0;
static int compressed = 0;
static char *compr_codec;

static struct option long_options[] =
{
    {"pcm", 0, 0, 'P'},
    {"debug", 0, 0, 'V'},
    {"Mmap", 0, 0, 'M'},
    {"HW", 1, 0, 'D'},
    {"Rate", 1, 0, 'R'},
    {"channel", 1, 0, 'C'},
    {"duration", 1, 0, 'T'},
    {"format", 1, 0, 'F'},
    {"period", 1, 0, 'B'},
    {"compressed", 1, 0, 'K'},
    {0, 0, 0, 0}
};

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

static int set_params(struct pcm *pcm)
{
     struct snd_pcm_hw_params *params;
     struct snd_pcm_sw_params *sparams;

     unsigned long periodSize, bufferSize, reqBuffSize;
     unsigned int periodTime, bufferTime;
     unsigned int requestedRate = pcm->rate;

     params = (struct snd_pcm_hw_params*) calloc(1, sizeof(struct snd_pcm_hw_params));
     if (!params) {
          fprintf(stderr, "Arec:Failed to allocate ALSA hardware parameters!");
          return -ENOMEM;
     }

     param_init(params);

     param_set_mask(params, SNDRV_PCM_HW_PARAM_ACCESS,
                    (pcm->flags & PCM_MMAP)? SNDRV_PCM_ACCESS_MMAP_INTERLEAVED : SNDRV_PCM_ACCESS_RW_INTERLEAVED);
     param_set_mask(params, SNDRV_PCM_HW_PARAM_FORMAT, pcm->format);
     param_set_mask(params, SNDRV_PCM_HW_PARAM_SUBFORMAT,
                    SNDRV_PCM_SUBFORMAT_STD);
     if (period)
         param_set_min(params, SNDRV_PCM_HW_PARAM_PERIOD_BYTES, period);
     else
         param_set_min(params, SNDRV_PCM_HW_PARAM_PERIOD_TIME, 10);
     param_set_int(params, SNDRV_PCM_HW_PARAM_SAMPLE_BITS, 16);
     param_set_int(params, SNDRV_PCM_HW_PARAM_FRAME_BITS,
                    pcm->channels * 16);
     param_set_int(params, SNDRV_PCM_HW_PARAM_CHANNELS,
                    pcm->channels);
     param_set_int(params, SNDRV_PCM_HW_PARAM_RATE, pcm->rate);

     param_set_hw_refine(pcm, params);

     if (param_set_hw_params(pcm, params)) {
         fprintf(stderr, "Arec:cannot set hw params");
         return -errno;
     }
     if (debug)
          param_dump(params);

     pcm->buffer_size = pcm_buffer_size(params);
     pcm->period_size = pcm_period_size(params);
     pcm->period_cnt = pcm->buffer_size/pcm->period_size;
     if (debug) {
        fprintf (stderr,"period_size (%d)", pcm->period_size);
        fprintf (stderr," buffer_size (%d)", pcm->buffer_size);
        fprintf (stderr," period_cnt  (%d)\n", pcm->period_cnt);
     }
     sparams = (struct snd_pcm_sw_params*) calloc(1, sizeof(struct snd_pcm_sw_params));
     if (!sparams) {
         fprintf(stderr, "Arec:Failed to allocate ALSA software parameters!\n");
         return -ENOMEM;
     }
    sparams->tstamp_mode = SNDRV_PCM_TSTAMP_NONE;
    sparams->period_step = 1;

    if (pcm->flags & PCM_MONO) {
        sparams->avail_min = pcm->period_size/2;
        sparams->xfer_align = pcm->period_size/2;
    } else if (pcm->flags & PCM_QUAD) {
        sparams->avail_min = pcm->period_size/8;
        sparams->xfer_align = pcm->period_size/8;
    } else if (pcm->flags & PCM_5POINT1) {
        sparams->avail_min = pcm->period_size/12;
        sparams->xfer_align = pcm->period_size/12;
    } else {
        sparams->avail_min = pcm->period_size/4;
        sparams->xfer_align = pcm->period_size/4;
    }

    sparams->start_threshold = 1;
    sparams->stop_threshold = INT_MAX;
    sparams->silence_size = 0;
    sparams->silence_threshold = 0;

    if (param_set_sw_params(pcm, sparams)) {
         fprintf(stderr, "Arec:cannot set sw params");
         return -errno;
    }
    if (debug) {
        fprintf (stderr,"avail_min (%lu)\n", sparams->avail_min);
        fprintf (stderr,"start_threshold (%lu)\n", sparams->start_threshold);
        fprintf (stderr,"stop_threshold (%lu)\n", sparams->stop_threshold);
        fprintf (stderr,"xfer_align (%lu)\n", sparams->xfer_align);
    }
    return 0;

}

int record_file(unsigned rate, unsigned channels, int fd, unsigned count,  unsigned flags, const char *device)
{
    unsigned xfer, bufsize, framesize;
    int r, avail;
    int nfds = 1;
    static int start = 0;
    struct snd_xferi x;
    long frames;
    unsigned offset = 0;
    int err;
    struct pollfd pfd[1];
    int rec_size = 0;
    framesize = 0;
    flags |= PCM_IN;

    if (channels == 1)
        flags |= PCM_MONO;
    else if (channels == 4)
        flags |= PCM_QUAD;
    else if (channels == 6)
        flags |= PCM_5POINT1;
    else
        flags |= PCM_STEREO;

    pcm = pcm_open(flags, device);
    if (!pcm_ready(pcm)) {
        pcm_close(pcm);
        goto fail;
    }

    if (compressed) {
       struct snd_compr_caps compr_cap;
       struct snd_compr_params compr_params;
       printf("SNDRV_COMPRESS_GET_CAPS= 0x%X\n", SNDRV_COMPRESS_GET_CAPS);
       if (ioctl(pcm->fd, SNDRV_COMPRESS_GET_CAPS, &compr_cap)) {
          fprintf(stderr, "AREC: SNDRV_COMPRESS_GET_CAPS, failed Error no %d \n", errno);
          pcm_close(pcm);
          return -errno;
       }
       if (!period)
           period = compr_cap.min_fragment_size;
           switch (get_compressed_format(compr_codec)) {
           case SND_AUDIOCODEC_MP3:
               compr_params.codec.id = SND_AUDIOCODEC_MP3;
               break;
#if 0
           case SND_AUDIOCODEC_AC3_PASS_THROUGH:
               compr_params.codec.id = SND_AUDIOCODEC_AC3_PASS_THROUGH;
               printf("codec -d = %x\n", compr_params.codec.id);
               break;
#endif
           case SND_AUDIOCODEC_AMRWB:
               compr_params.codec.id = SND_AUDIOCODEC_AMRWB;
               compr_params.codec.options.generic.reserved[0] = 8; /*band mode - 23.85 kbps*/
               compr_params.codec.options.generic.reserved[1] = 0; /*dtx mode - disable*/
               printf("codec -d = %x\n", compr_params.codec.id);
               break;
           default:
               break;
           }
       if (ioctl(pcm->fd, SNDRV_COMPRESS_SET_PARAMS, &compr_params)) {
          fprintf(stderr, "AREC: SNDRV_COMPRESS_SET_PARAMS,failed Error no %d \n", errno);
          pcm_close(pcm);
          return -errno;
       }
    }
    pcm->channels = channels;
    pcm->rate = rate;
    pcm->flags = flags;
    pcm->format = format;
    if (set_params(pcm)) {
        fprintf(stderr, "Arec:params setting failed\n");
        pcm_close(pcm);
        return -EINVAL;
    }

    if (!pcm_flag) {
        if (pcm_prepare(pcm)) {
            fprintf(stderr, "Arec:Failed in pcm_prepare\n");
            pcm_close(pcm);
            return -errno;
        }
	if (ioctl(pcm->fd, SNDRV_PCM_IOCTL_START)) {
            fprintf(stderr, "Arec: Hostless IOCTL_START Error no %d \n", errno);
            pcm_close(pcm);
            return -errno;
	}
        while(1) {
            sleep(10);
	}
   }

    if (flags & PCM_MMAP) {
        u_int8_t *dst_addr = NULL;
        struct snd_pcm_sync_ptr *sync_ptr1 = pcm->sync_ptr;
        unsigned int tmp;

        if (mmap_buffer(pcm)) {
             fprintf(stderr, "Arec:params setting failed\n");
             pcm_close(pcm);
             return -EINVAL;
        }
        if (debug)
            fprintf(stderr, "Arec:mmap_buffer done\n");

        if (pcm_prepare(pcm)) {
            fprintf(stderr, "Arec:Failed in pcm_prepare\n");
            pcm_close(pcm);
            return -errno;
        }

        bufsize = pcm->period_size;

        if (debug)
            fprintf(stderr, "Arec:bufsize = %d\n", bufsize);
        if (ioctl(pcm->fd, SNDRV_PCM_IOCTL_START)) {
            if (errno == EPIPE) {
			fprintf(stderr, "Arec:Failed in SNDRV_PCM_IOCTL_START\n");
			/* we failed to make our window -- try to restart */
			pcm->running = 0;
            } else {
                fprintf(stderr, "Arec:Error no %d \n", errno);
                return -errno;
            }
        }

        pfd[0].fd = pcm->fd;
        pfd[0].events = POLLIN;

        hdr.data_sz = 0;
        if (pcm->flags & PCM_MONO) {
                frames = bufsize / 2;
        } else if (pcm->flags & PCM_QUAD) {
                frames = bufsize / 8;
        } else if (pcm->flags & PCM_5POINT1) {
                frames = bufsize / 12;
        } else{
                frames = bufsize / 4;
        }
        x.frames = frames;
        for(;;) {
            if (!pcm->running) {
                if (pcm_prepare(pcm))
                    return --errno;
                start = 0;
            }
            /* Sync the current Application pointer from the kernel */
            pcm->sync_ptr->flags = SNDRV_PCM_SYNC_PTR_APPL | SNDRV_PCM_SYNC_PTR_AVAIL_MIN;/*SNDRV_PCM_SYNC_PTR_HWSYNC;*/
            err = sync_ptr(pcm);
            if (err == EPIPE) {
                fprintf(stderr, "Arec:Failed in sync_ptr \n");
                /* we failed to make our window -- try to restart */
                //pcm->overruns++;
                pcm->running = 0;
                continue;
                }
               /*
                * Check for the available data in driver. If available data is
                * less than avail_min we need to wait
                */
                avail = pcm_avail(pcm);
                if (debug)
                     fprintf(stderr, "Arec:avail 1 = %d frames = %ld, avail_min = %d,"
                             "x.frames = %d, bufsize = %d, dst_addr = %p\n",avail, frames,
                             (int)pcm->sw_p->avail_min, (int)x.frames, bufsize, dst_addr);
                if (avail < 0)
                        return avail;
                if (avail < pcm->sw_p->avail_min) {
                        poll(pfd, nfds, TIMEOUT_INFINITE);
                        continue;
                }
                if (x.frames > avail)
                        frames = avail;
               /*
                * Now that we have data size greater than avail_min available to
                * to be read we need to calcutate the buffer offset where we can
                * start reading from.
                */
                dst_addr = dst_address(pcm);
                if (compressed) {
                    framesize =  (unsigned)(dst_addr[3] << 24) + (unsigned)(dst_addr[2] << 16) +
                        (unsigned) (dst_addr[1] << 8) + (unsigned)dst_addr[0];
                    if (debug)
                        fprintf(stderr, "Arec:dst_addr[0] = %d, dst_addr[1] = %d,"
                            "dst_addr[2] = %d, dst_addr[3] = %d, dst_addr[4] = %d,"
                            "dst_addr[5] = %d, dst_addr = %p, bufsize = %d, framesize = %d\n",
                            dst_addr[0], dst_addr[1], dst_addr[2], dst_addr[3], dst_addr[4],
                            dst_addr[5],dst_addr,  bufsize, framesize);
                    dst_addr += COMPR_META_DATA_SIZE;
                } else {
                    framesize = bufsize;
                }

               /*
                * Write to the file at the destination address from kernel mmaped buffer
                * This reduces a extra copy of intermediate buffer.
                */
                if (write(fd, dst_addr, framesize) != framesize) {
                    fprintf(stderr, "Arec:could not write %d bytes\n", framesize);
                    return -errno;
                }
                x.frames -= frames;
                pcm->sync_ptr->c.control.appl_ptr += frames;
                pcm->sync_ptr->flags = 0;
                err = sync_ptr(pcm);
                if (err == EPIPE) {
                     fprintf(stderr, "Arec:Failed in sync_ptr \n");
                     /* we failed to make our window -- try to restart */
                     pcm->running = 0;
                     continue;
                }
                rec_size += bufsize;
                if (!compressed) {
                    hdr.data_sz += bufsize;
                    hdr.riff_sz = hdr.data_sz + 44 - 8;
                    if (!piped) {
                        lseek(fd, 0, SEEK_SET);
                        write(fd, &hdr, sizeof(hdr));
                        lseek(fd, 0, SEEK_END);
                    }
                }
                if (rec_size >= count)
                      break;
           }
    } else {
	    bufsize = pcm->period_size;
            if (pcm_prepare(pcm)) {
                fprintf(stderr, "Arec:Failed in pcm_prepare\n");
                pcm_close(pcm);
                return -errno;
            }

	    data = calloc(1, bufsize);
	    if (!data) {
		fprintf(stderr, "Arec:could not allocate %d bytes\n", bufsize);
		return -ENOMEM;
	    }

	    while (!pcm_read(pcm, data, bufsize)) {
		if (write(fd, data, bufsize) != bufsize) {
		    fprintf(stderr, "Arec:could not write %d bytes\n", bufsize);
		    break;
		}
                rec_size += bufsize;
                hdr.data_sz += bufsize;
                hdr.riff_sz = hdr.data_sz + 44 - 8;
                if (!piped) {
                    lseek(fd, 0, SEEK_SET);
                    write(fd, &hdr, sizeof(hdr));
                    lseek(fd, 0, SEEK_END);
                }
                if (rec_size >= count)
                    break;
	    }
    }
    fprintf(stderr, " rec_size =%d count =%d\n", rec_size, count);
    close(fd);
    free(data);
    pcm_close(pcm);
    return hdr.data_sz;

fail:
    fprintf(stderr, "Arec:pcm error: %s\n", pcm_error(pcm));
    return -errno;
}

int rec_raw(const char *fg, const char *device, int rate, int ch,
                    const char *fn)
{
    unsigned flag = 0;
    uint32_t rec_max_sz = 2147483648LL;
    uint32_t count;
    int i = 0;
    printf("rec_raw-> pcm_flag = %d\n", pcm_flag);
    if (!fn) {
        fd = fileno(stdout);
        piped = 1;
    } else {
        fd = open(fn, O_WRONLY | O_CREAT | O_TRUNC, 0664);
        if (fd < 0) {
            fprintf(stderr, "Arec:arec: cannot open '%s'\n", fn);
            return -EBADFD;
        }
    }
    if (duration == 0) {
         count = rec_max_sz;
    } else {
         count = rate * ch * 2;
         count *= (uint32_t)duration;
    }
    count = count < rec_max_sz ? count : rec_max_sz;
    if (debug)
        fprintf(stderr, "arec: %d ch, %d hz, %d bit, format %x\n",
        ch, rate, 16, format);

    if (!strncmp(fg, "M", sizeof("M"))) {
        flag = PCM_MMAP;
    } else if (!strncmp(fg, "N", sizeof("N"))) {
        flag = PCM_NMMAP;
    }
    return record_file(rate, ch, fd, count, flag, device);
}

int rec_wav(const char *fg, const char *device, int rate, int ch, const char *fn)
{
    unsigned flag = 0;
    uint32_t rec_max_sz = 2147483648LL;
    uint32_t count = 0;
    int i = 0;
    printf("rec_wav-> pcm_flag = %d\n", pcm_flag);
    if (pcm_flag) {
        if (!fn) {
            fd = fileno(stdout);
            piped = 1;
        } else {
            fd = open(fn, O_WRONLY | O_CREAT | O_TRUNC, 0664);
            if (fd < 0) {
                fprintf(stderr, "Arec:arec: cannot open '%s'\n", fn);
                return -EBADFD;
            }
        }
        if (compressed) {

            printf("rec_wav-> compressed = %d\n", compressed);
            hdr.sample_rate = rate;
            hdr.num_channels = ch;
            goto ignore_header;
        }
        memset(&hdr, 0, sizeof(struct wav_header));
        hdr.riff_id = ID_RIFF;
        hdr.riff_fmt = ID_WAVE;
        hdr.fmt_id = ID_FMT;
        hdr.fmt_sz = 16;
        hdr.audio_format = FORMAT_PCM;
        hdr.num_channels = ch;
        hdr.sample_rate = rate;
        hdr.bits_per_sample = 16;
        hdr.byte_rate = (rate * ch * hdr.bits_per_sample) / 8;
        hdr.block_align = ( hdr.bits_per_sample * ch ) / 8;
        hdr.data_id = ID_DATA;
        hdr.data_sz = 0;
        hdr.riff_sz = hdr.data_sz + 44 - 8;
        if (write(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
            if (debug)
                fprintf(stderr, "arec: cannot write header\n");
            return -errno;
        }
        if (debug)
            fprintf(stderr, "arec: %d ch, %d hz, %d bit, %s\n",
                    hdr.num_channels, hdr.sample_rate, hdr.bits_per_sample,
                    hdr.audio_format == FORMAT_PCM ? "PCM" : "unknown");
    } else {
        hdr.sample_rate = rate;
        hdr.num_channels = ch;
    }
ignore_header:

   if (duration == 0) {
        count = rec_max_sz;
    } else {
          count = rate * ch * 2;
          count *= (uint32_t)duration;
    }
    if (!strncmp(fg, "M", sizeof("M"))) {
        flag = PCM_MMAP;
    } else if (!strncmp(fg, "N", sizeof("N"))) {
        flag = PCM_NMMAP;
    }
    return record_file(hdr.sample_rate, hdr.num_channels, fd, count, flag, device);
}

static void signal_handler(int sig)
{
    long file_size;
    FILE *fp;

    fprintf(stderr, "Arec:Aborted by signal %s...\n", strsignal(sig));
    fprintf(stderr, "Arec:lseeked to %d", (int) lseek(fd, 0, SEEK_SET));
    hdr.riff_sz = hdr.data_sz + 44 - 8;
    fprintf(stderr, "Arec: hdr.data_sz =%d\n", hdr.data_sz);
    fprintf(stderr, "Arec: hdr.riff_sz =%d\n", hdr.riff_sz);
    if (write(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
	if (debug)
            fprintf(stderr, "Arec:arec: cannot write header\n");
    } else
       fd = -1;

    if (fd > 1) {
        close(fd);
        fd = -1;
    }
    free(filename);
    free(data);
    pcm = NULL;
    raise(sig);
}

int main(int argc, char **argv)
{
    int rate = 48000;
    int ch = 1;
    int i = 0;
    int option_index = 0;
    int c;
    char *mmap = "N";
    char *device = "hw:0,0";
    struct sigaction sa;
    int rc = 0;
    char *cformat = NULL;

    if (argc < 2) {
        printf("\nUsage: arec [options] <file>\n"
              "options:\n"
              "-D <hw:C,D>        -- Alsa PCM by name\n"
              "-M     -- Mmap stream\n"
              "-P     -- Hostless steam[No PCM]\n"
              "-V     -- verbose\n"
              "-C     -- Channels\n"
              "-R     -- Rate\n"
              "-T     -- Time in seconds for recording\n"
              "-F     -- Format\n"
              "-B     -- Period\n"
              "-K <AC3,DTS,etc>   -- compressed\n"
              "<file> \n");
           for (i = 0; i < SNDRV_PCM_FORMAT_LAST; ++i) {
                cformat = get_format_name(i);
               if (cformat != NULL)
                   fprintf(stderr, "%s ", cformat);
           }
           fprintf(stderr, "\nSome of these may not be available on selected hardware\n");
          return 0;
    }
    while ((c = getopt_long(argc, argv, "PVMD:R:C:T:F:B:K:", long_options, &option_index)) != -1) {
       switch (c) {
       case 'P':
          pcm_flag = 0;
          break;
       case 'V':
          debug = 1;
          break;
       case 'M':
          mmap = "M";
          break;
       case 'D':
          device = optarg;
          break;
       case 'R':
          rate = (int)strtol(optarg, NULL, 0);
          break;
       case 'C':
          ch  = (int)strtol(optarg, NULL, 0);
          break;
       case 'T':
          duration = (int)strtol(optarg, NULL, 0);
          break;
       case 'F':
          format = (int)get_format(optarg);
          break;
       case 'B':
          period = (int)strtol(optarg, NULL, 0);
          break;
       case 'K':
          compressed = 1;
          printf("compressed codec type requested = %s\n", optarg);
          compr_codec = optarg;
          break;
       default:
          printf("\nUsage: arec [options] <file>\n"
                "options:\n"
                "-D <hw:C,D>        -- Alsa PCM by name\n"
                "-M     -- Mmap stream\n"
                "-P     -- Hostless steam[No PCM]\n"
                "-V     -- verbose\n"
                "-C     -- Channels\n"
                "-R     -- Rate\n"
                "-T     -- Time in seconds for recording\n"
                "-F     -- Format\n"
                "-B     -- Period\n"
                "-K <AC3,DTS,etc>   -- compressed\n"
                "<file> \n");
           for (i = 0; i < SNDRV_PCM_FORMAT_LAST; ++i) {
                cformat = get_format_name(i);
               if (cformat != NULL)
                   fprintf(stderr, "%s ", cformat);
           }
           fprintf(stderr, "\nSome of these may not be available on selected hardware\n");
          return -EINVAL;
       }
    }
    filename = (char*) calloc(1, 30);
     if (!filename) {
          fprintf(stderr, "Arec:Failed to allocate filename!");
          return -ENOMEM;
    }
    if (optind > argc - 1) {
        free(filename);
        filename = NULL;
    } else {
        //strlcpy(filename, argv[optind++], 30);
        strncpy(filename, argv[optind++], 30);
    }

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &signal_handler;
    sigaction(SIGABRT, &sa, NULL);

    if (pcm_flag) {
        if (format == SNDRV_PCM_FORMAT_S16_LE)
             rc = rec_wav(mmap, device, rate, ch, filename);
         else
             rc = rec_raw(mmap, device, rate, ch, filename);
    } else {
        rc = rec_wav(mmap, device, rate, ch, "dummy");
    }
    if (filename)
        free(filename);

    return rc;
}

