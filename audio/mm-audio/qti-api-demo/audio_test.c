/*#define LOG_NDEBUG 0*/
#include <fcntl.h>
#include <linux/netlink.h>
#include <getopt.h>
#include <pthread.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <utils/Log.h>
#include <math.h>

#include <cutils/list.h>
#include "qahw_api.h"
#include "qahw_defs.h"
#include "qlaudio_api.h"
static FILE * log_file = NULL;
void usage();
int main(int argc, char *argv[]) {

    int status = 0;
    uint32_t play_duration_in_seconds = 600, sink_device = 2;
    uint32_t source_device = AUDIO_DEVICE_IN_HDMI;
	static float loopback_gain = 0;
	
	log_file = stdout;
    struct option long_options[] = {
        /* These options set a flag. */
        {"sink-device", required_argument,    0, 'o'},
        {"source-device", required_argument,    0, 'i'},
        {"play-duration",  required_argument,    0, 'p'},
        {"play-volume",  required_argument,    0, 'v'},
        {"help",          no_argument,          0, 'h'},
        {0, 0, 0, 0}
    };

    int opt = 0;
    int option_index = 0;
    struct Audio_Playback pl;

    while ((opt = getopt_long(argc,
                              argv,
                              "-o:i:p:v:h",
                              long_options,
                              &option_index)) != -1) {


        switch (opt) {
        case 'o':
            sink_device = atoll(optarg);
            break;
        case 'i':
            source_device = atoll(optarg);
            break;
        case 'p':
            play_duration_in_seconds = atoi(optarg);
            break;
        case 'v':
            loopback_gain = atof(optarg);
            break;
        case 'h':
        default :
            usage();
            return 0;
            break;
        }
    }
	pl.channel = 1;
	pl.filetype = 2;
	pl.device = 2;
	pl.volume = 50;
	pl.rate = 48000;	
 	set_audio_test_mode(loopback_gain);
    QL_Audio_Playback(pl,"/data/ten.mp3");	   
    return 0;
}

void usage()
{
   
 }
