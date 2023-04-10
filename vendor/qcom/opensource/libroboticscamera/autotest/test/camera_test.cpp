/* Copyright (c) 2015-2017 The Linux Foundataion. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*************************************************************************
*
*Application Notes:  Refer readme.md
*
****************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <syslog.h>
#include <inttypes.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/stat.h>
#include <algorithm>

#include "camera.h"
#include "camera_parameters.h"
#include "omx_video.h"

#if defined(_HAL3_CAMERA_)
#define DEFAULT_EXPOSURE_VALUE  5000000 // ns
#define MIN_EXPOSURE_VALUE 0
#define MAX_EXPOSURE_VALUE 33000000
#define DEFAULT_GAIN_VALUE  300
#define MIN_GAIN_VALUE 0
#define MAX_GAIN_VALUE  1000
#define DEFAULT_EXPOSURE_TIME_VALUE "0"
#else
#define DEFAULT_EXPOSURE_VALUE 250
#define MIN_EXPOSURE_VALUE 1
#define MAX_EXPOSURE_VALUE 65535
#define DEFAULT_GAIN_VALUE  300
#define MIN_GAIN_VALUE 256
#define MAX_GAIN_VALUE 2048
#define DEFAULT_EXPOSURE_TIME_VALUE "0"
#endif

#define QCAMERA_DUMP_LOCATION "/data/misc/camera/dumps/"
#define QCAMERA_SAVE_LOCATION "/data/misc/camera/"

#define DEFAULT_CAMERA_FPS 30
#define MS_PER_SEC 1000
#define NS_PER_MS 1000000
#define NS_PER_US 1000
#define MAX_BUF_SIZE 128

const int SNAPSHOT_WIDTH_ALIGN = 64;
const int SNAPSHOT_HEIGHT_ALIGN = 64;
const int TAKEPICTURE_TIMEOUT_MS = 5000;

#define QT_PREVIEW_WIDTH 640
#define QT_PREVIEW_HEIGHT 480
static void *eHandle = NULL;
static FILE *h264_file = NULL;
static char vfile_name[255];

using namespace std;
using namespace camera;

struct CameraCaps
{
    vector<ImageSize> pSizes, vSizes, picSizes,rawPicSizes;
    vector<string> focusModes, wbModes, isoModes;
    vector<string> sharpnessEdgeModes, tonemap_modes;
    Range brightness, sharpness, contrast;
    vector<Range> previewFpsRanges;
    vector<VideoFPS> videoFpsValues;
    vector<string> previewFormats;
    string rawSize;
    Range64 exposureRange;
    Range gainRange;
};

enum OutputFormatType{
    YUV_FORMAT,
    RAW_FORMAT,
    JPEG_FORMAT
};

enum CamFunction {
    CAM_FUNC_HIRES = 0,             //hi-res
    CAM_FUNC_LEFT_SENSOR = 3,       //left ov
    CAM_FUNC_TRACKING = 1,          //tracking ov
    CAM_FUNC_RIGHT_SENSOR = 4,      //right ov
    CAM_FUNC_STEREO = 2,            // _todo, stereo
    CAM_FUNC_TOF = 5,
    CAM_FUNC_MAX,
};

enum AppLoglevel {
    CAM_LOG_SILENT = 0,
    CAM_LOG_ERROR = 1,
    CAM_LOG_INFO = 2,
    CAM_LOG_DEBUG = 3,
    CAM_LOG_MAX,
};

/**
*  Helper class to store all parameter settings
*/
struct TestConfig
{
    bool dumpFrames;
    bool infoMode;
    bool testSnapshot;
    bool testVideo;
    int runTime;
    int exposureValue;
    int gainValue;
    string expTimeValue;
    CamFunction func;
    OutputFormatType outputFormat;
    OutputFormatType snapshotFormat;
    ImageSize pSize;
    ImageSize vSize;
    ImageSize picSize;
    int fps;
    AppLoglevel logLevel;
    int storagePath;
    int statsLogMask;
    string focusModeStr;
    uint32_t num_images;
    string antibanding;
    string isoMode;
    string wbMode;
    int32_t sharpnessValue;
    bool testContrastToneMap;
};

/**
 * CLASS  CameraTest
 *
 * - inherits ICameraListers which provides core functionality
 * - User must define onPreviewFrame (virtual) function. It is
 *    the callback function for every preview frame.
 * - If user is using VideoStream then the user must define
 *    onVideoFrame (virtual) function. It is the callback
 *    function for every video frame.
 * - If any error occurs,  onError() callback function is
 *    called. User must define onError if error handling is
 *    required.
 */
class CameraTest : ICameraListener
{
public:

    CameraTest();
    CameraTest(TestConfig config);
    ~CameraTest();
    int run();

    int initialize(int camId);

    /* listener methods */
    virtual void onError();
    virtual void onPreviewFrame(ICameraFrame* frame);
    virtual void onVideoFrame(ICameraFrame* frame);
    virtual void onPictureFrame(ICameraFrame* frame);

private:
    ICameraDevice* camera_;
    CameraParams params_;
    ImageSize pSize_, vSize_, picSize_;
    CameraCaps caps_;
    TestConfig config_;

    uint32_t vFrameCount_, pFrameCount_;
    float vFpsAvg_, pFpsAvg_;

    uint64_t vTimeStampPrev_, pTimeStampPrev_;

    pthread_cond_t cvPicDone;
    pthread_mutex_t mutexPicDone;
    bool isPicDone;

    int printCapabilities();
    int setParameters();
    int takePicture(uint32_t num_images = 1);
    int setFPSindex(TestConfig& cfg, int &pFpsIdx, int &vFpsIdx);
    void* recorderInit(int camId);
};

CameraTest::CameraTest() :
    camera_(NULL),
    vFrameCount_(0),
    pFrameCount_(0),
    vFpsAvg_(0.0f),
    pFpsAvg_(0.0f),
    vTimeStampPrev_(0),
    pTimeStampPrev_(0)
{
    pthread_cond_init(&cvPicDone, NULL);
    pthread_mutex_init(&mutexPicDone, NULL);
}

CameraTest::CameraTest(TestConfig config) :
    camera_(NULL),
    vFrameCount_(0),
    pFrameCount_(0),
    vFpsAvg_(0.0f),
    pFpsAvg_(0.0f),
    vTimeStampPrev_(0),
    pTimeStampPrev_(0)
{
    config_ = config;
    pthread_cond_init(&cvPicDone, NULL);
    pthread_mutex_init(&mutexPicDone, NULL);
}

int CameraTest::initialize(int camId)
{
    int rc;
    rc = ICameraDevice::createInstance(camId, &camera_);//open camera here, sofia marked for study
    if (rc != 0) {
        printf("could not open camera %d, rc %d\n", camId, rc);
        return rc;
    }
    camera_->addListener(this);

    rc = params_.init(camera_);
    if (rc != 0) {
        printf("failed to init parameters\n");
        ICameraDevice::deleteInstance(&camera_);
        return rc;
    }
    //printf("params = %s\n", params_.toString().c_str());
    /* query capabilities */
    caps_.pSizes = params_.getSupportedPreviewSizes();
    caps_.vSizes = params_.getSupportedVideoSizes();
    caps_.picSizes = params_.getSupportedPictureSizes(FORMAT_JPEG);
    caps_.rawPicSizes = params_.getSupportedPictureSizes(FORMAT_RAW10);
    caps_.focusModes = params_.getSupportedFocusModes();
    caps_.wbModes = params_.getSupportedWhiteBalance();
    caps_.sharpnessEdgeModes= params_.getSupportedSharpnessMode();
    caps_.tonemap_modes= params_.getSupportedToneMapMode();
    caps_.isoModes = params_.getSupportedISO();
    caps_.brightness = params_.getSupportedBrightness();
    caps_.sharpness = params_.getSupportedSharpness();
    caps_.contrast = params_.getSupportedContrast();
    caps_.previewFpsRanges = params_.getSupportedPreviewFpsRanges();
    caps_.videoFpsValues = params_.getSupportedVideoFps();
    caps_.previewFormats = params_.getSupportedPreviewFormats();
    caps_.rawSize = params_.get("raw-size");
    caps_.exposureRange = params_.getManualExposureRange(config_.pSize, config_.fps);
    caps_.gainRange = params_.getManualGainRange(config_.pSize, config_.fps);
    return 0;
}

CameraTest::~CameraTest()
{
}

static int dumpToFile(uint8_t* data, uint32_t size, char* name, uint64_t timestamp)
{
    FILE* fp;
    fp = fopen(name, "wb");
    if (!fp) {
        printf("fopen failed for %s\n", name);
        return -1;
    }
    fwrite(data, size, 1, fp);
    printf("saved filename %s\n", name);
    fclose(fp);
    return 0;
}

static inline uint32_t align_size(uint32_t size, uint32_t align)
{
    return ((size + align - 1) & ~(align-1));
}

int CameraTest::takePicture(uint32_t num_images)
{
    int rc;
    pthread_mutex_lock(&mutexPicDone);
    isPicDone = false;
    printf("take picture\n");
    rc = camera_->takePicture(num_images);
    if (rc) {
        printf("takePicture failed\n");
        pthread_mutex_unlock(&mutexPicDone);
        return rc;
    }

    struct timespec waitTime;
    struct timeval now;

    gettimeofday(&now, NULL);
    waitTime.tv_sec = now.tv_sec + TAKEPICTURE_TIMEOUT_MS/MS_PER_SEC;
    waitTime.tv_nsec = now.tv_usec * NS_PER_US + (TAKEPICTURE_TIMEOUT_MS % MS_PER_SEC) * NS_PER_MS;
    /* wait for picture done */
    while (isPicDone == false) {
        rc = pthread_cond_timedwait(&cvPicDone, &mutexPicDone, &waitTime);
        if (rc == ETIMEDOUT) {
            printf("error: takePicture timed out\n");
            break;
        }
    }
    pthread_mutex_unlock(&mutexPicDone);
    return 0;
}

void CameraTest::onError()
{
    printf("camera error!, aborting\n");
    exit(EXIT_FAILURE);
}


string getStringFromEnum(CamFunction e)
{
  switch(e)
  {
  case CAM_FUNC_HIRES: return "0";
  case CAM_FUNC_TRACKING: return "1";
  case CAM_FUNC_LEFT_SENSOR: return "left";
  case CAM_FUNC_RIGHT_SENSOR: return "right";
  case CAM_FUNC_STEREO: return "stereo";
  case CAM_FUNC_TOF: return "tof";
  default:
      printf("error: unknown camera type \n");
  }

  return "unknown";
}

/**
 *
 * FUNCTION: onPreviewFrame
 *
 *  - This is called every frame I
 *  - In the test app, we save files only after every 30 frames
 *  - In parameter frame (ICameraFrame) also has the timestamps
 *    field which is public
 *
 * @param frame
 *
 */
void CameraTest::onPreviewFrame(ICameraFrame* frame)
{

    uint64_t diff;
    int ret;

    ret = pthread_mutex_trylock(&mutexPicDone);
    if(EBUSY == ret){
        printf("taking picture, return\n");
        return;
    }else{
        if(!isPicDone){
            printf("is taking picture, return\n");
            pthread_mutex_unlock(&mutexPicDone);
            return;
        }
    }

    diff = frame->timeStamp - pTimeStampPrev_;
    pFpsAvg_ = ((pFpsAvg_ * pFrameCount_) + (1e9 / diff)) / (pFrameCount_ + 1);
    pFrameCount_++;
    pTimeStampPrev_  = frame->timeStamp;
#if defined(_HAL3_CAMERA_)
    static uint32_t printOnceCompletedflag=false;
#endif

    if (pFrameCount_ > 0 && pFrameCount_ % 30 == 0) {
        char name[MAX_BUF_SIZE];

        if ( config_.outputFormat == RAW_FORMAT )
        {
            if(config_.storagePath ==1){
                    snprintf(name, MAX_BUF_SIZE, QCAMERA_DUMP_LOCATION "P_%dx%d_%04d_%llu_%s.raw",
                 pSize_.width, pSize_.height, pFrameCount_,frame->timeStamp,getStringFromEnum(config_.func).c_str());
            }else
            snprintf(name, MAX_BUF_SIZE, "P_%dx%d_%04d_%llu_%s.raw",
                 pSize_.width, pSize_.height, pFrameCount_,frame->timeStamp,getStringFromEnum(config_.func).c_str());
        }else{
            if(config_.storagePath ==1){
                    snprintf(name, MAX_BUF_SIZE, QCAMERA_DUMP_LOCATION "P_%dx%d_%04d_%llu_%s.yuv",
                 pSize_.width, pSize_.height, pFrameCount_,frame->timeStamp,getStringFromEnum(config_.func).c_str());
            }else{
             snprintf(name, MAX_BUF_SIZE, "P_%dx%d_%04d_%llu_%s.yuv",
                 pSize_.width, pSize_.height, pFrameCount_,frame->timeStamp,getStringFromEnum(config_.func).c_str());
            }

            frame->size = (pSize_.width * pSize_.height * 3) / 2;
        }

        if (config_.dumpFrames == true) {
            dumpToFile(frame->data, frame->size, name, frame->timeStamp);
        }
#if defined(_HAL3_CAMERA_)
        if ( !printOnceCompletedflag ){
                printf("%s:%d F_ts = %lld skew = %lld , Readout ts = %lld , duration = %lld\n",__func__,__LINE__,  \
                                                                     frame->timeStamp,  \
                                                                     params_.getFrameRollingShutterSkew(frame),  \
                                                                     params_.getFrameReadoutTimestamp(frame),  \
                                                                     params_.getFrameReadoutDuration(frame));
                printOnceCompletedflag = true;
        }
        //printf("Preview FPS = %.2f\n", pFpsAvg_);
        printf("%s:%d F_ts = %lld exposure time = %lld gain = %d\n",__func__,__LINE__,  \
                                                                     frame->timeStamp,  \
                                                                     params_.getFrameExposureTime(frame),  \
                                                                     params_.getFrameGainValue(frame));

#endif
    }
    pthread_mutex_unlock(&mutexPicDone);
}

void CameraTest::onPictureFrame(ICameraFrame* frame)
{
    char imageName[MAX_BUF_SIZE];
	struct timeval tv;
	gettimeofday(&tv, NULL);

    if (config_.snapshotFormat == RAW_FORMAT) {
        if(config_.storagePath ==1){
                snprintf(imageName, MAX_BUF_SIZE, QCAMERA_DUMP_LOCATION "snapshot_mipi_raw10_%dx%d_%lld_%s.raw", picSize_.width, picSize_.height,frame->timeStamp,getStringFromEnum(config_.func).c_str());
        }else
                snprintf(imageName, MAX_BUF_SIZE, "snapshot_mipi_raw10_%dx%d_%lld_%s.raw", picSize_.width, picSize_.height,frame->timeStamp,getStringFromEnum(config_.func).c_str());

    } else {
        if(config_.storagePath ==1){
                snprintf(imageName, MAX_BUF_SIZE, QCAMERA_DUMP_LOCATION "snapshot_%dx%d_%lld_%s.jpg", picSize_.width, picSize_.height,frame->timeStamp,getStringFromEnum(config_.func).c_str());
        }else
                snprintf(imageName, MAX_BUF_SIZE, QCAMERA_SAVE_LOCATION "snapshot_%d_%d_cam%s_%ld.jpg", picSize_.width, picSize_.height, getStringFromEnum(config_.func).c_str(),tv.tv_sec);
    }


    dumpToFile(frame->data, frame->size, imageName, frame->timeStamp);
    /* notify the waiting thread about picture done */
    pthread_mutex_lock(&mutexPicDone);
    isPicDone = true;
    pthread_cond_signal(&cvPicDone);
    pthread_mutex_unlock(&mutexPicDone);
    printf("%s:%d\n", __func__, __LINE__);
}

/**
 *
 * FUNCTION: onVideoFrame
 *
 *  - This is called every frame I
 *  - In the test app, we save files only after every 30 frames
 *  - In parameter frame (ICameraFrame) also has the timestamps
 *    field which is public
 *
 * @param frame
 *
 */
void CameraTest::onVideoFrame(ICameraFrame* frame)
{
	int length = 0;
	unsigned char *buffer;
	unsigned char *eBuffer = NULL;
    buffer = (unsigned char *)malloc(QT_PREVIEW_WIDTH*QT_PREVIEW_HEIGHT*3/2);

    memcpy(buffer, (unsigned char *)frame->data, QT_PREVIEW_WIDTH*QT_PREVIEW_HEIGHT);
    memcpy(buffer + (QT_PREVIEW_WIDTH*QT_PREVIEW_HEIGHT), (unsigned char *)(frame->data + (QT_PREVIEW_WIDTH*QT_PREVIEW_HEIGHT)), QT_PREVIEW_WIDTH*QT_PREVIEW_HEIGHT/2);

	eBuffer = omx_encoder_process(eHandle, (unsigned char *)buffer, QT_PREVIEW_WIDTH*QT_PREVIEW_HEIGHT*3/2, frame->timeStamp, &length);

	if (h264_file != NULL) {
		if (eBuffer != NULL && length != 0) {
			fwrite(eBuffer, 1, length, h264_file);

		}
	}
	free(buffer);
}

int CameraTest::printCapabilities()
{
    printf("Camera capabilities\n");

    printf("available preview sizes:\n");
    for (size_t i = 0; i < caps_.pSizes.size(); i++) {
        printf("%d: %d x %d\n", i, caps_.pSizes[i].width, caps_.pSizes[i].height);
    }
    printf("available video sizes:\n");
    for (size_t i = 0; i < caps_.vSizes.size(); i++) {
        printf("%d: %d x %d\n", i, caps_.vSizes[i].width, caps_.vSizes[i].height);
    }
    printf("available jpeg picture sizes:\n");
    for (size_t i = 0; i < caps_.picSizes.size(); i++) {
        printf("%d: %d x %d\n", i, caps_.picSizes[i].width, caps_.picSizes[i].height);
    }
    printf("available raw picture sizes:\n");
    for (size_t i = 0; i < caps_.rawPicSizes.size(); i++) {
        printf("%d: %d x %d\n", i, caps_.rawPicSizes[i].width, caps_.rawPicSizes[i].height);
    }
    printf("available preview formats:\n");
    for (size_t i = 0; i < caps_.previewFormats.size(); i++) {
        printf("%d: %s\n", i, caps_.previewFormats[i].c_str());
    }
    printf("available focus modes:\n");
    for (size_t i = 0; i < caps_.focusModes.size(); i++) {
        printf("%d: %s\n", i, caps_.focusModes[i].c_str());
    }
    printf("available whitebalance modes:\n");
    for (size_t i = 0; i < caps_.wbModes.size(); i++) {
        printf("%d: %s\n", i, caps_.wbModes[i].c_str());
    }
    printf("available ISO modes:\n");
    for (size_t i = 0; i < caps_.isoModes.size(); i++) {
        printf("%d: %s\n", i, caps_.isoModes[i].c_str());
    }
    printf("available brightness values:\n");
    printf("min=%d, max=%d, step=%d\n", caps_.brightness.min,
           caps_.brightness.max, caps_.brightness.step);

    printf("available Sharpness EDGE modes:\n");
    for (size_t i = 0; i < caps_.sharpnessEdgeModes.size(); i++) {
        printf("%d: %s\n", i, caps_.sharpnessEdgeModes[i].c_str());
    }
    printf("available Tone Map modes (for contrast on HAL3):\n");
    for (size_t i = 0; i < caps_.tonemap_modes.size(); i++) {
        printf("%d: %s\n", i, caps_.tonemap_modes[i].c_str());
    }

    printf("available sharpness values:\n");
    printf("min=%d, max=%d, step=%d\n", caps_.sharpness.min,
           caps_.sharpness.max, caps_.sharpness.step);

    printf("available contrast values:\n");
    printf("min=%d, max=%d, step=%d\n", caps_.contrast.min,
           caps_.contrast.max, caps_.contrast.step);

    printf("available preview fps ranges:\n");
    for (size_t i = 0; i < caps_.previewFpsRanges.size(); i++) {
        printf("%d: [%d, %d]\n", i, caps_.previewFpsRanges[i].min,
               caps_.previewFpsRanges[i].max);
    }
    printf("available video fps values:\n");
    for (size_t i = 0; i < caps_.videoFpsValues.size(); i++) {
        printf("%d: %d\n", i, caps_.videoFpsValues[i]);
    }

    printf("available manual exposure range: min = %lld , max = %lld \n", caps_.exposureRange.min, caps_.exposureRange.max);
    printf("available manual gain range: min = %d , max = %d \n", caps_.gainRange.min, caps_.gainRange.max);

    return 0;
}
#define TOF_IRS_WIDTH 224
#define TOF_IRS_HEIGHT 172

ImageSize UHDSize(3840,2160);
ImageSize FIVEMegaSize(2592,1944);
ImageSize FHDSize(1920,1080);
ImageSize HDSize(1280,720);
ImageSize VGASize(640,480);
ImageSize TOF_1_PHASE_SIZE(TOF_IRS_WIDTH,TOF_IRS_HEIGHT);
ImageSize stereoVGASize(1280, 480);
ImageSize QVGASize(320,240);
ImageSize stereoQVGASize(640,240);

/**
 * FUNCTION: setFPSindex
 *
 * scans through the supported fps values and returns index of
 * requested fps in the array of supported fps
 *
 * @param fps      : Required FPS  (Input)
 * @param pFpsIdx  : preview fps index (output)
 * @param vFpsIdx  : video fps index   (output)
 *
 *  */
int CameraTest::setFPSindex(TestConfig & cfg, int &pFpsIdx, int &vFpsIdx)
{
    int defaultPrevFPSIndex = -1;
    int defaultVideoFPSIndex = -1;
    size_t i;
    int rc = 0;
    int preview_fps = cfg.fps;
    if (cfg.testVideo == true) {
        preview_fps = DEFAULT_CAMERA_FPS;
    }
    for (i = 0; i < caps_.previewFpsRanges.size(); i++) {
        printf("caps_.previewFpsRanges[i] %d, preview_fps %d\n", caps_.previewFpsRanges[i].max/1000, preview_fps);
        if (  (caps_.previewFpsRanges[i].max)/1000 == preview_fps )
        {
            pFpsIdx = i;
            break;
        }
        if ( (caps_.previewFpsRanges[i].max)/1000 == DEFAULT_CAMERA_FPS )
        {
            defaultPrevFPSIndex = i;
        }
    }
    if ( i >= caps_.previewFpsRanges.size() )
    {
        if (defaultPrevFPSIndex != -1 )
        {
            pFpsIdx = defaultPrevFPSIndex;
        } else
        {
            pFpsIdx = -1;
            rc = -1;
        }
    }

    for (i = 0; i < caps_.videoFpsValues.size(); i++) {
        printf("caps_.videoFpsValues[i] %d, video fps %d\n", caps_.videoFpsValues[i], cfg.fps);
        if ( cfg.fps == 30 * caps_.videoFpsValues[i])
        {
            vFpsIdx = i;
            break;
        }
        if ( DEFAULT_CAMERA_FPS == 30 * caps_.videoFpsValues[i])
        {
            defaultVideoFPSIndex = i;
        }
    }
    if ( i >= caps_.videoFpsValues.size())
    {
        if (defaultVideoFPSIndex != -1)
        {
            vFpsIdx = defaultVideoFPSIndex;
        }else
        {
            vFpsIdx = -1;
            rc = -1;
        }
    }
    return rc;
}
/**
 *  FUNCTION : setParameters
 *
 *  - When camera is opened, it is initialized with default set
 *    of parameters.
 *  - This function sets required parameters based on camera and
 *    usecase
 *  - params_setXXX and params_set  only updates parameter
 *    values in a local object.
 *  - params_.commit() function will update the hardware
 *    settings with the current state of the parameter object
 *  - Some functionality will not be application for all for
 *    sensor modules. for eg. tracking camera sensor does not support
 *    autofocus/focus mode.
 *  - Reference setting for different sensors and format are
 *    provided in this function.
 *
 *  */
int CameraTest::setParameters()
{
    size_t index;
    int focusModeIdx = 0;
    int pFpsIdx = 3;
    int vFpsIdx = 3;
    vector<ImageSize> supportedSnapshotSizes;
#if defined(_HAL3_CAMERA_)
    Tonemap_RBG tonemapCurves;
#endif

    pSize_ = config_.pSize;
    vSize_ = config_.vSize;
    picSize_ = config_.picSize;
    printf("config_.func %d\n", config_.func);
    switch ( config_.func ){
        case CAM_FUNC_TRACKING:
            if (config_.outputFormat == RAW_FORMAT) {
                /* Do not turn on videostream for tracking camera in RAW format */
                config_.testVideo = false;
                printf("Setting output = RAW_FORMAT for tracking camera sensor \n");
                #if defined(_HAL3_CAMERA_)
                    params_.setPreviewFormat(FORMAT_RAW10);
                #else
                    params_.set("preview-format", "bayer-rggb");
                    params_.set("picture-format", "bayer-mipi-10gbrg");
                    params_.set("raw-size", "640x480");
                #endif
            }
            if (config_.snapshotFormat == JPEG_FORMAT) {
                params_.setPictureSize(picSize_);
                params_.setPictureThumbNailSize(picSize_);
                printf("Setting snapshotFormat = JPEG_FORMAT for tracking camera %d x %d \n",
                       picSize_.width,picSize_.height);
            }
        break;
        case CAM_FUNC_TOF:
            if (config_.outputFormat == RAW_FORMAT) {
                /* Do not turn on videostream for tof camera in RAW format */
                config_.testVideo = false;
                printf("Setting output = RAW_FORMAT for tof camera sensor \n");
                #if defined(_HAL3_CAMERA_)
                    params_.setPreviewFormat(FORMAT_RAW12);
                #else
                    params_.set("preview-format", "bayer-mipi-12bggr");
                    params_.set("raw-size", "224x172");
                #endif
            }
            break;
        case CAM_FUNC_LEFT_SENSOR:
            if (config_.outputFormat == RAW_FORMAT) {
                /* Do not turn on videostream for tracking camera in RAW format */
                config_.testVideo = false;
                printf("Setting output = RAW_FORMAT for tracking camera sensor \n");
                #if defined(_HAL3_CAMERA_)
                    params_.setPreviewFormat(FORMAT_RAW10);
                #else
                    params_.set("preview-format", "bayer-rggb");
                    params_.set("picture-format", "bayer-mipi-10gbrg");
                    params_.set("raw-size", "640x480");
                #endif
            }
            break;
        case CAM_FUNC_RIGHT_SENSOR:
            if (config_.outputFormat == RAW_FORMAT) {
                /* Do not turn on videostream for tracking camera in RAW format */
                config_.testVideo = false;
                printf("Setting output = RAW_FORMAT for tracking camera sensor \n");
                #if defined(_HAL3_CAMERA_)
                    params_.setPreviewFormat(FORMAT_RAW10);
                #else
                    params_.set("preview-format", "bayer-rggb");
                    params_.set("picture-format", "bayer-mipi-10gbrg");
                    params_.set("raw-size", "640x480");
                #endif
              }
              break;
         case CAM_FUNC_STEREO:
                break;
         case CAM_FUNC_HIRES:
            {
                if (config_.snapshotFormat == RAW_FORMAT) {
                    printf("Setting snapshot format : raw \n");
                    #if defined(_HAL3_CAMERA_)
                        params_.setPictureFormat(FORMAT_RAW10);
                    #else
                        params_.set("picture-format","bayer-mipi-10bggr");
                    #endif
                    printf("raw picture size: %s\n", caps_.rawSize.c_str());
                    supportedSnapshotSizes = caps_.rawPicSizes;
                } else {
                    printf("Setting snapshot format : jpeg \n");
                    #if defined(_HAL3_CAMERA_)
                        params_.setPictureFormat(FORMAT_JPEG);
                    #else
                        //Default is jpeg
                    #endif
                    supportedSnapshotSizes = caps_.picSizes;
                }
                // Check requested snapshot resolution is supported for requested format
                if (config_.picSize.width == 99999 && config_.picSize.height == 99999 ) {
                    if (supportedSnapshotSizes.size() == 0) {
                        printf("Error: No snapshot resolution found for requested format \n");
                        exit(1);
                    }
                    picSize_ = supportedSnapshotSizes[0];
                } else {
                    for ( index = 0 ; index < supportedSnapshotSizes.size() ; index++) {
                        if ( config_.picSize.width == supportedSnapshotSizes[index].width
                             && config_.picSize.height == supportedSnapshotSizes[index].height)
                        {
                            picSize_ = supportedSnapshotSizes[index];
                            break;
                        }
                    }
                    if ( index >= supportedSnapshotSizes.size() ) {
                        printf("Error: Snapshot resolution %d x %d not supported for requested format \n"
                                ,config_.picSize.width,config_.picSize.height);
                        exit(1);
                    }
                }

                params_.setPictureSize(picSize_);
                printf("Setting snapshot size : %d x %d \n", picSize_.width, picSize_.height );

                if (config_.snapshotFormat == JPEG_FORMAT) {
                    params_.setPictureThumbNailSize(picSize_);
                }

                printf("%s:%d \n",__func__,__LINE__);
                for (size_t i = 0; i < caps_.focusModes.size(); i++) {
                       //printf("inside for loop:%d:%s \n",caps_.focusModes.size(),caps_.focusModes[i].c_str());
                       if(strcmp(caps_.focusModes[i].c_str(),config_.focusModeStr.c_str()) == 0){
                           //focusModeIdx=i;
                        }
                }
                printf("setting focus mode: idx = %d , str = %s\n",
                         focusModeIdx,caps_.focusModes[focusModeIdx].c_str());


                if ( std::find(caps_.focusModes.begin(), caps_.focusModes.end(), config_.focusModeStr.c_str()) != caps_.focusModes.end()){
                   params_.setFocusMode(config_.focusModeStr);
                   printf("%s:%d setting focusMode = %s \n",__func__,__LINE__,config_.focusModeStr.c_str());
               }
               else{
                   printf("%s:%d Requested focusMode = %s not available \n",__func__,__LINE__,config_.focusModeStr.c_str());
               }


               if ( std::find(caps_.isoModes.begin(), caps_.isoModes.end(), config_.isoMode.c_str()) != caps_.isoModes.end()){
                   params_.setISO(config_.isoMode);
                   printf("%s:%d setting isoMode = %s \n",__func__,__LINE__,config_.isoMode.c_str());
               }
               else{
                   printf("%s:%d Requested isoMode = %s not available \n",__func__,__LINE__,config_.isoMode.c_str());
               }

               if ( std::find(caps_.wbModes.begin(), caps_.wbModes.end(), config_.wbMode.c_str()) != caps_.wbModes.end()){
                   params_.setWhiteBalance(config_.wbMode);
                   printf("%s:%d setting wbMode = %s \n",__func__,__LINE__,config_.wbMode.c_str());
               }
               else{
                   printf("%s:%d Requested wbMode = %s not available \n",__func__,__LINE__,config_.wbMode.c_str());
               }

               if (config_.sharpnessValue != -1) {
                   if (config_.sharpnessValue >= caps_.sharpness.min && config_.sharpnessValue <= caps_.sharpness.max){
                       #if defined(_HAL3_CAMERA_)
                       params_.setSharpnessMode(SHARPNESS_MODE_EDGE_HIGH_QUALITY);
                       #endif
                       params_.setSharpness(config_.sharpnessValue);
                       printf("%s:%d setting sharpness = %d \n",__func__,__LINE__,config_.sharpnessValue);
                   }else {
                       printf("%s:%d Requested sharpness value = %d not in range\n",
                                                 __func__,__LINE__,config_.sharpnessValue);
                   }
               }

               #if defined(_HAL3_CAMERA_)
               /* Reverse value of each RGB color channel */
               if (config_.testContrastToneMap == true) {

                   for (int i = 0 ; i < 3 ; i++) {
                       tonemapCurves.tonemap_points_cnt = 2;
                       tonemapCurves.curves[i].tonemap_points[0][0] =  0;
                       tonemapCurves.curves[i].tonemap_points[0][1] =  1;
                       tonemapCurves.curves[i].tonemap_points[1][0] =  1;
                       tonemapCurves.curves[i].tonemap_points[1][1] =  0;
                   }

                   params_.setToneMapMode(TONEMAP_CONTRAST_CURVE);
                   params_.setContrastTone(tonemapCurves);

                   tonemapCurves = params_.getContrastTone();
                   for (size_t i = 0 ; i < tonemapCurves.tonemap_points_cnt ; i++) {
                        printf("%s:%d ContrastToneMap : Pin:Pout  %f  - %f   \n",
                             __func__,__LINE__,
                             tonemapCurves.curves[0].tonemap_points[i][0],
                             tonemapCurves.curves[0].tonemap_points[i][1] );
                   }
               }
               #endif
            }
            break;
        default:
            printf("invalid sensor function \n");
            break;
    }

    printf("setting preview size: %dx%d\n", pSize_.width, pSize_.height);
    params_.setPreviewSize(pSize_);

#if defined(_HAL3_CAMERA_)
    if ( CAM_FUNC_TOF != config_.func) {
#endif
        printf("setting video size: %dx%d\n", vSize_.width, vSize_.height);
        params_.setVideoSize(vSize_);
#if defined(_HAL3_CAMERA_)
    }
#endif

    /* Find index and set FPS  */
    int rc = setFPSindex(config_, pFpsIdx, vFpsIdx);
    if ( rc == -1){
        return rc;
    }

    printf("setting preview fps range: %d, %d ( idx = %d ) \n",
        caps_.previewFpsRanges[pFpsIdx].min,
        caps_.previewFpsRanges[pFpsIdx].max, pFpsIdx);
    params_.setPreviewFpsRange(caps_.previewFpsRanges[pFpsIdx]);

    printf("setting video fps: %d ( idx = %d )\n", caps_.videoFpsValues[vFpsIdx], vFpsIdx );
    params_.setVideoFPS(caps_.videoFpsValues[vFpsIdx]);

    printf("setting antibanding: %s\n",config_.antibanding.c_str());
    params_.setAntibanding(config_.antibanding.c_str());

    params_.setStatsLoggingMask(config_.statsLogMask);

    return params_.commit();
}

void* CameraTest::recorderInit(int camId)
{
	struct EncoderInitParam eInitParam;
	eInitParam.width = 640;
    eInitParam.height = 480;
    eInitParam.bitrate = 1024*1024;
    eInitParam.framerate = 30;
    eInitParam.ratetype = 0;
    eInitParam.intraPeriod = 30;
    eInitParam.minQP = 15;
    eInitParam.maxQP = 30;
    eInitParam.codecProfileType = CODECPROFILE_AVC_Baseline;
    eInitParam.codecType = CODEC_AVC;
    eHandle = (void *) omx_encoder_init(eInitParam);
	
	struct timeval tv;
	gettimeofday(&tv, NULL);
	
	if (eHandle == NULL) {
		printf("encorder init, eHandle null\n");
	} else {
		//printf("cam%d init success\n",camId);
	}

	if (camId == 0) {
		sprintf(vfile_name, "/data/misc/camera/video_640_480_cam0_%ld.h264", tv.tv_sec);
		h264_file = fopen(vfile_name, "w+");
	} else {
		sprintf(vfile_name, "/data/misc/camera/video_640_480_cam1_%ld.h264", tv.tv_sec);
		h264_file = fopen(vfile_name, "w+");
	}
	return eHandle;
}

int CameraTest::run()
{
    int rc = EXIT_SUCCESS;
	int passCamId = 0;
    /* returns the number of camera-modules connected on the board */
    int n = getNumberOfCameras();

    if (n < 0) {
        printf("getNumberOfCameras() failed, rc=%d\n", n);
        return EXIT_FAILURE;
    }

    printf("num_cameras = %d\n", n);

    if (n < 1) {
        printf("No cameras found.\n");
        return EXIT_FAILURE;
    }

    int camId = -1;
    if (config_.func < CAM_FUNC_MAX){
#if defined(_HAL3_CAMERA_)
        camId = (int)config_.func;
#else
        /* find camera based on function */
        for (int i=0; i<n; i++) {
            CameraInfo info;
            getCameraInfo(i, info);
            printf(" i = %d , info.func = %d \n",i, info.func);
            if (info.func == config_.func) {
                camId = i;
            }
        }
#endif
    }else {
        camId = -1;
        printf("Camera not found \n");
        exit(1);
    }

#if defined(_HAL3_CAMERA_)
    // Hack : for TOF sensor set camId = 0
    // Assumption: Only TOF sensor is connected (required)
    if (CAM_FUNC_TOF == config_.func){
            camId = 0;
            printf("Testing TOF camera sesnor id=%d\n", camId);
    }
#endif

    printf("Testing camera id=%d\n", camId);

    rc = initialize(camId);
    if (0 != rc) {
        return rc;
    }

    if (config_.infoMode) {
        printCapabilities();
        return rc;
    }

    rc = setParameters();
    if (rc) {
        printf("setParameters failed\n");
        goto del_camera;
    }

    /* initialize perf counters */
    vFrameCount_ = 0;
    pFrameCount_ = 0;
    vFpsAvg_ = 0.0f;
    pFpsAvg_ = 0.0f;


    /* starts the preview stream. At every preview frame onPreviewFrame( ) callback is invoked */
    if ((config_.fps <= DEFAULT_CAMERA_FPS) || (config_.testVideo == false)) {
        printf("start preview\n");
        camera_->startPreview();
    }

    /* Set parameters which are required after starting preview */
    switch(config_.func)
    {
        case CAM_FUNC_TRACKING:
            {
                 params_.setManualExposure(config_.exposureValue);
                 params_.setManualGain(config_.gainValue);
				 passCamId = 1;
                 printf("Setting exposure value =  %d , gain value = %d \n", config_.exposureValue, config_.gainValue );
            }
            break;
        case CAM_FUNC_TOF:
            {
                 //_TODO:
            }
            break;

        case CAM_FUNC_RIGHT_SENSOR:
            {
                 params_.setManualExposure(config_.exposureValue);
                 params_.setManualGain(config_.gainValue);
                 params_.setVerticalFlip(true);
                 params_.setHorizontalMirror(true);
                 printf("Setting exposure value =  %d , gain value = %d \n", config_.exposureValue, config_.gainValue );
            }
            break;
        case CAM_FUNC_LEFT_SENSOR:
            {
                 params_.setManualExposure(config_.exposureValue);
                 params_.setManualGain(config_.gainValue);
                 params_.setVerticalFlip(true);
                 params_.setHorizontalMirror(true);
                 printf("Setting exposure value =  %d , gain value = %d \n", config_.exposureValue, config_.gainValue );
            }
            break;
        case CAM_FUNC_STEREO:
            {
                params_.setManualExposure(config_.exposureValue);
                params_.setManualGain(config_.gainValue);
                printf("Setting exposure value =  %d , gain value = %d \n", config_.exposureValue, config_.gainValue );
                params_.setVerticalFlip(true);
                params_.setHorizontalMirror(true);
                printf("Setting Vertical Flip and Horizontal Mirror bit in sensor \n");
            }
            break;
        case CAM_FUNC_HIRES:
            {
                params_.setExposureTime(config_.expTimeValue);
                params_.setISO(config_.isoMode);
				passCamId = 0;
                printf("Setting exposure time value = %s , iso value = %s\n",
                    config_.expTimeValue.c_str(), config_.isoMode.c_str());
            }
            break;
        case CAM_FUNC_MAX:
        default:
            {
                rc = EXIT_FAILURE;
                printf("Invalid camera cofig type \n");
                goto del_camera;
            }

    }
    rc = params_.commit();
    if (rc) {
        printf("commit failed\n");
        exit(EXIT_FAILURE);
    }

    if (config_.testVideo  == true ) {
        /* starts video stream. At every video frame onVideoFrame( )  callback is invoked */
        printf("start recording\n");
		recorderInit(passCamId);
        camera_->startRecording();
    }

    if (config_.testSnapshot == true) {
        printf("waiting for 2 seconds for exposure to settle...\n");
        /* sleep required to settle the exposure before taking snapshot.
           This app does not provide interactive feedback to user
           about the exposure */
        sleep(2);
        printf("taking picture\n");
        rc = takePicture(config_.num_images);

        if (rc) {
            printf("takePicture failed\n");
            exit(EXIT_FAILURE);
        }
    }


    /* Put the main/run thread to sleep and process the frames in the callbacks */
    printf("waiting for %d seconds ...\n", config_.runTime);
    sleep(config_.runTime);


    /* After the sleep interval stop preview stream, stop video stream and end application */
    if (config_.testVideo  == true) {
        printf("stop recording\n");
        camera_->stopRecording();
    }

    printf("stop preview\n");
    camera_->stopPreview();

    //printf("Average preview FPS = %.2f\n", pFpsAvg_);
    if( config_.testVideo  == true ) {
        //printf("Average video FPS = %.2f\n", vFpsAvg_);
		//printf("omx_encoder_release\n");
		omx_encoder_release(eHandle);
		if (h264_file) {
			fclose(h264_file);
		}
	}

del_camera:
    /* release camera device */
    ICameraDevice::deleteInstance(&camera_);
    return rc;
}

/**
 *  FUNCTION: setDefaultConfig
 *
 *  set default config based on camera module
 *
 * */
static int setDefaultConfig(TestConfig &cfg) {

    cfg.outputFormat = YUV_FORMAT;
    cfg.dumpFrames = false;
    cfg.runTime = 5;
    cfg.infoMode = false;
    cfg.testVideo = false;
    cfg.testSnapshot = false;
    cfg.exposureValue = DEFAULT_EXPOSURE_VALUE;  /* Default exposure value */
    cfg.gainValue = DEFAULT_GAIN_VALUE;  /* Default gain value */
    cfg.expTimeValue = DEFAULT_EXPOSURE_TIME_VALUE;
    cfg.fps = DEFAULT_CAMERA_FPS;
    cfg.logLevel = CAM_LOG_DEBUG;
    cfg.snapshotFormat = JPEG_FORMAT;
    cfg.statsLogMask = STATS_NO_LOG;
    cfg.focusModeStr = FOCUS_MODE_OFF;
    cfg.storagePath = 0;
    cfg.num_images = 1;
    cfg.antibanding = "off";
    cfg.isoMode = ISO_AUTO;
    cfg.wbMode = WHITE_BALANCE_AUTO;
    cfg.sharpnessValue = -1;
    cfg.testContrastToneMap = false;

    switch (cfg.func) {
    case CAM_FUNC_TRACKING:
        cfg.pSize   = VGASize;
        cfg.vSize   = VGASize;
        cfg.picSize   = VGASize;
        break;
    case CAM_FUNC_TOF:
        cfg.pSize   = TOF_1_PHASE_SIZE;
        cfg.vSize   = TOF_1_PHASE_SIZE;
        cfg.picSize = TOF_1_PHASE_SIZE;
        cfg.outputFormat = RAW_FORMAT;
        break;
    case CAM_FUNC_RIGHT_SENSOR:
        cfg.pSize   = VGASize;
        cfg.vSize   = VGASize;
        cfg.picSize   = VGASize;
        break;
    case CAM_FUNC_LEFT_SENSOR:
        cfg.pSize   = VGASize;
        cfg.vSize   = VGASize;
        cfg.picSize   = VGASize;
        break;
    case CAM_FUNC_STEREO:
        cfg.pSize = stereoVGASize;
        cfg.vSize  = stereoVGASize;
        cfg.picSize  = stereoVGASize;
        break;
    case CAM_FUNC_HIRES:
        cfg.pSize   = VGASize;
        cfg.vSize   = VGASize;
        cfg.picSize   = VGASize;
        break;
    default:
        printf("invalid sensor function \n");
        break;
    }
    return 0;
}

/**
 *  FUNCTION: parseCommandline
 *
 *  parses commandline options and populates the config
 *  data structure
 *
 *  */
static TestConfig parseCommandline(int argc, char* argv[])
{
    TestConfig cfg;
    cfg.func = CAM_FUNC_HIRES;

	if(strcmp(argv[1],"0")== 0 ){
		cfg.func = CAM_FUNC_HIRES;
	}else if(strcmp(argv[1],"1")== 0){
		cfg.func = CAM_FUNC_TRACKING;
	}else {
		printf("error input arg1\n");
		return cfg;
	}

    setDefaultConfig(cfg);

	if(strcmp(argv[2],"snapshot") == 0){
		cfg.testSnapshot = true;
	}else if(strcmp(argv[2],"video") == 0){
		cfg.testVideo = true;
	}else{
		printf("error input arg2\n");
		return cfg;
	}

#ifndef _HAL3_CAMERA_
    if (cfg.snapshotFormat == RAW_FORMAT) {
        cfg.testVideo = false;
    }
#endif
    return cfg;
}

int main(int argc, char* argv[])
{
    TestConfig config = parseCommandline(argc, argv);

    /* setup syslog level */
    if (config.logLevel == CAM_LOG_SILENT) {
        setlogmask(LOG_UPTO(LOG_EMERG));
    } else if (config.logLevel == CAM_LOG_DEBUG) {
        setlogmask(LOG_UPTO(LOG_DEBUG));
    } else if (config.logLevel == CAM_LOG_INFO) {
        setlogmask(LOG_UPTO(LOG_INFO));
    } else if (config.logLevel == CAM_LOG_ERROR) {
        setlogmask(LOG_UPTO(LOG_ERR));
    }
    openlog(NULL, LOG_NDELAY, LOG_DAEMON);

    CameraTest test(config);
    test.run();

    return EXIT_SUCCESS;
}
