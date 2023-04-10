/*
 *   Copyright:  (C) 2020 Quectel. All rights reserved.
 *    Filename:  ql_power_manager.c
 * Description:  This file 
 *        Version:  1.0.0(20200930)
 *         Author:  Peeta/fulinux <peeta.chen@quectel.com>
 *      ChangeLog:  1, Release initial version on 20200930
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/reboot.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <linux/input.h>
#include <cutils/properties.h>

#include <wifi/ql_wifi.h>
#include <ql-mcm-api/ql_in.h>
#include <RilPublic.h>
#include <ql_misc.h>

#define KEY_PWK_EVENT_FILE_NAME     "/dev/input/event0"
#define LCD_BACKLIGHT_BRIGHTNESS    "/sys/class/leds/lcd-backlight/brightness"
#define POWER_STATE_FILE_NAME       "/sys/power/state"
#define ANDROID_USB_STATE_FILE_NAME "/sys/class/android_usb/android0/state"
#define MCM_SERVICE_READY_FILE      "/tmp/mcm_service_ready.flag"
#define POWER_MANAGER_CONFIG_FILE   "/systemrw/ql_power_manager.conf"
#define PROC_CMDLINE_FILE_NAME      "/proc/cmdline"

#define KEY_PWK     116
#define EVENT_NUM   64
#define LOW_POWER_MODE_ON   1
#define LOW_POWER_MODE_OFF  0

static int qlril_handle = 0;
static int screen_handle = 0;
static int s_pipeReadFd = 0;
static int s_pipeWriteFd = 0;

static nw_client_handle_type h_nw = 0;
static voice_client_handle_type h_voice = 0;

static int autoSleepFlag = 0;
static int ffbmModeFlag = 0;
static int lcdOff_time = 5;
static int suspend_time = 1;
static int gotolcdoff_time = 0;
static int gotosleep_time = 0;

static int cur_lcd_state = 0;
static int cur_usb_state = 0;
static int cur_wifi_state = 0;
static unsigned char cur_brightness = 255;
static unsigned char def_brightness = 255;

static int voiceCallFlag = 0;
static int voiceCallActive = 0;
static char number[100] = {0};

static pthread_mutex_t s_mutex = PTHREAD_MUTEX_INITIALIZER;

static int QL_Sync_Power_Manager_Conf(void)
{
    int ret;
    FILE *fp;
    char *ptr;
    char buf[100];
    int count = 0;
    static struct stat state_new, state_old;

    if (access(POWER_MANAGER_CONFIG_FILE, F_OK)) {
        return -1;
    }

    ret =stat(POWER_MANAGER_CONFIG_FILE, &state_new);
    if (state_new.st_mtime != state_old.st_mtime) {
        state_old.st_mtime = state_new.st_mtime;
        fp = fopen(POWER_MANAGER_CONFIG_FILE, "r");
        if (fp == NULL) {
            perror("fopen ql_power_manager");
            return -2;
        }

        while(fgets(buf, sizeof(buf) - 1, fp) != NULL) {
            ptr = strchr(buf, '=');
            if (!strncmp(buf, "AUTO_SLEEP_SWITCH", ptr - buf)) {
                sscanf(ptr + 1, "%d%*s", &ret);
                if (autoSleepFlag == ret)
                    continue;
                autoSleepFlag = ret;
                printf("%s:%d: AUTO_SLEEP_SWITCH = %d\n", __func__, __LINE__, autoSleepFlag);
                count++;
            }

            if (!strncmp(buf, "LCD_TURN_OFF_TIME_S", ptr - buf)) {
                sscanf(ptr + 1, "%d%*s", &ret);
                if (lcdOff_time == ret)
                    continue;
                lcdOff_time = (ret < 5)? 5:ret;
                printf("%s:%d: LCD_TURN_OFF_TIME_S = %d\n", __func__, __LINE__, lcdOff_time);
                count++;
            }

            if (!strncmp(buf, "SYSTEM_SUSPEND_TIME_S", ptr - buf)) {
                sscanf(ptr + 1, "%d%*s", &ret);
                if (suspend_time == ret)
                    continue;
                suspend_time = (ret < 1)? 1:ret;
                printf("%s:%d: SYSTEM_SUSPEND_TIME_S = %d\n", __func__, __LINE__, suspend_time);
                count++;
            }

            if (!strncmp(buf, "DEFAULT_BRIGHTNESS", ptr - buf)) {
                sscanf(ptr + 1, "%d%*s", &ret);
                if (def_brightness == ret)
                    continue;
                def_brightness = (ret > 255)? 255:ret;
                def_brightness = (ret < 0)? 0:ret;
                printf("%s:%d: DEFAULT_BRIGHTNESS = %d\n", __func__, __LINE__, def_brightness);
                count++;
            }
            memset(buf, 0, sizeof(buf));
        }

        fclose(fp);
    }

    return count;
}

static void QL_Voice_Call_Ind_Func(unsigned int ind_id, void* ind_data, uint32_t ind_data_len)
{
    int ret;
    int i = 0;
    int state = 0;

    if (NULL == ind_data) {
        return;
    }

    switch (ind_id) {
        case E_QL_MCM_VOICE_CALL_IND:
        {
            if (ind_data_len != sizeof(ql_mcm_voice_call_ind)) {
                break;;
            }

            ql_mcm_voice_call_ind *pVoiceCallInd = (ql_mcm_voice_call_ind*)ind_data;

            for(i = 0; i < pVoiceCallInd->calls_len; i++) {
                int end_reason_idx = 0;
                if(pVoiceCallInd->calls[i].call_end_reason_valid) {
                    end_reason_idx = pVoiceCallInd->calls[i].call_end_reason;
                } else {
                    end_reason_idx = 0;
                }
                printf("###### Call id=%d, PhoneNum:%s, event=%d! ######\n",
                            pVoiceCallInd->calls[i].call_id,
                            pVoiceCallInd->calls[i].number,
                            pVoiceCallInd->calls[i].state);

                state = pVoiceCallInd->calls[i].state;
                if ((state == 0 || state == 1) && voiceCallFlag == 0) {//not END
                    voiceCallFlag = 1;//只进来一次
                    ret = write(s_pipeWriteFd, "ON", 3);
                    if (ret < 0) {
                        perror("write error");
                    }
                } else if (state == 3) {
                    snprintf(number, sizeof(number), "%s", pVoiceCallInd->calls[i].number);
                    voiceCallActive = 1;
                } else if (state == 5) {
                    if (voiceCallActive == 1) {
                        if (strcmp(number, pVoiceCallInd->calls[i].number) == 0) {
                            voiceCallActive = 0;
                        }
                    }

                    voiceCallFlag = 0;
                    ret = write(s_pipeWriteFd, "RST", 4);
                    if (ret < 0) {
                        perror("write error");
                    }
                }
            }

            break;
        }
        default:
            break;
    }
}

void processUnsolInd_cb(int *handle, int slotId, int event, void *data, size_t size)
{
    int ret = 0;
    int i, num;
    int state = 0;

    if (NULL == data)
        return;

    switch(event) {
        case RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED: //1001
        {
            RIL_Call *calls = NULL;
            ret = QLRIL_GetCurrentCalls(handle, slotId, (void **)&calls);
            if (ret >= 0 && calls != NULL) {
                num = ret;
                for (i = 0; i < num; i++) {
                    if (calls[i].isVoice == 0)
                        continue;

                    state = calls[i].state;
                    if ((state == 4 || state == 2 || state == 3) && voiceCallFlag == 0) {
                        voiceCallFlag = 1;//只进来一次
                        ret = write(s_pipeWriteFd, "ON", 3);
                        if (ret < 0) {
                            perror("write error");
                        }
                    } else if (state == 0 && voiceCallActive == 0) {
                        snprintf(number, sizeof(number), "%s", calls[i].number);
                        voiceCallActive = 1;
                    }
                }
            } else {
                voiceCallFlag = 0;
                voiceCallActive = 0;
                ret = write(s_pipeWriteFd, "RST", 4);
                if (ret < 0) {
                    perror("write error");
                }
            }
            break;
        }
        default:
            break;
    }
}

static int ql_get_wifi_status(void)
{
    char *ptr;
    char buf[500] = {0};
    char str[20] = {0};

    memset(buf, 0, sizeof(buf));
    memset(str, 0, sizeof(str));
    QL_WIFI_status(NULL, buf, sizeof(buf));

    ptr = strstr(buf, "wpa_state=");
    if (!ptr) {
        return -1;
    }

    sscanf(ptr, "wpa_state=%[^\n]\n", str);
    //printf("Peeta: wpa_state=%s\n", str);
    if (strncasecmp(str, "COMPLETED", 9) == 0)
        return 1;

    return 0;
}

static void *handle_system_sleep_thread(void *args)
{
    int i;
    int ret;
    char string[4] = {0};
    char buf[40] = {0};
    int times = 0;
    int times2 = 0;
    int low_power_mode = 0;
    int qlril_callback_flag = 0;
    int mcm_callback_flag = 0;

    property_set("sys.usb.connect", "0");
    property_set("ctrl.screen.power", "none");
    property_set("ctrl.sleep.now", "0");

    while(1) {
        if (screen_handle == 0) {
            ret = QLSCREEN_Init(&screen_handle);
            if (ret < 0 || screen_handle == 0) {
                printf("QLSCREEN_Init failure with return:%d\n", ret);
                screen_handle = 0;
            }
        }

		ret = times2 % 5;
		if ((h_nw == 0 || h_voice == 0) && ret == 0) {
			ret = QLMISC_IsRunning("mcm_ril_service");
			if (ret > 0) {
				if (h_nw == 0) {
					if(!access(MCM_SERVICE_READY_FILE, F_OK)) {
						ret = QL_MCM_NW_Client_Init(&h_nw);
						//printf ("fulinux %d ret = %d\n", __LINE__, ret);
						if (ret != 0 || h_nw <= 0) {
							printf("QL_MCM_NW_Client_Init failure with return:%d, h_nw=%d\n", ret, h_nw);
							h_nw = 0;
						}
					}
				}

				if (h_voice == 0) {
					if(!access(MCM_SERVICE_READY_FILE, F_OK)) {
						ret = QL_Voice_Call_Client_Init(&h_voice);
						if (ret != 0 || h_voice <= 0) {
							printf("QL_Voice_Call_Client_Init ret = %d, with h_voice=%d\n", ret, h_voice);
							h_voice = 0;
						}
					}
				} else if (qlril_callback_flag == 0 && mcm_callback_flag == 0) {
					ret = QL_Voice_Call_AddCommonStateHandler(h_voice,
							(QL_VoiceCall_CommonStateHandlerFunc_t)QL_Voice_Call_Ind_Func);
					if (ret == 0) {
						mcm_callback_flag = 1;
					}
				}
			} else {
				h_nw = 0;
				h_voice = 0;
				mcm_callback_flag = 0;
			}
		}

		ret = times2 % 5;
		if (qlril_handle == 0 && ret == 0) {
			property_get("sys.qlrild.ready", string, "0");
			ret = atoi(string);
			if (ret == 1) {
				ret = QLMISC_IsRunning("qlrild");
				if (ret > 0) {
					ret = QLRIL_Init(&qlril_handle);
					if (ret != 0) {
						printf("QLRIL_Init failure with return:%d\n", ret);
						qlril_handle = 0;
						qlril_callback_flag = 0;
					} else if (mcm_callback_flag == 0 && qlril_callback_flag == 0) {
						ret = QLRIL_AddUnsolEventsListener(&qlril_handle, processUnsolInd_cb, NULL);
						if (ret == 0) {
							ret = QLRIL_RegisterUnsolEvents(&qlril_handle, 1001, 1001);
							if (ret != 0) {
								printf("QLRIL_RegisterUnsolEvents failure\n");
								QLRIL_Exit(&qlril_handle);
								qlril_handle = 0;
							} else {
								printf("QLRIL_RegisterUnsolEvents success\n");
								qlril_callback_flag = 1;
							}
						} else {
							printf("QLRIL_AddUnsolEventsListener failure\n");
							QLRIL_Exit(&qlril_handle);
							qlril_handle = 0;
						}
					}
				} else {
					qlril_handle = 0;
					qlril_callback_flag = 0;
					property_set("sys.qlrild.ready", "0");
				}
			}
		}

		times2++;
		if (times2 > 1000)
			times2 = 0;

        if (s_pipeWriteFd != 0) {
            property_get("ctrl.sleep.now", string, "0");
            ret = atoi(string);
            property_get("ctrl.screen.power", string, "none");
            if ((strncasecmp(string, "on", 2) == 0)) {
                ret = write(s_pipeWriteFd, "ON", 3);
                if (ret < 0) {
                    perror("write error");
                }
                property_set("ctrl.screen.power", "none");
            } else if (!strncasecmp(string, "off", 3) || ret == 1) {
                ret = write(s_pipeWriteFd, "OFF", 4);
                if (ret < 0) {
                    perror("write error");
                }
                property_set("ctrl.sleep.now", "0");
                property_set("ctrl.screen.power", "none");
            }
        }

		if (cur_lcd_state == 0 || (times%5 == 1))
			cur_usb_state = QLMISC_USBCableState();
#if 0
        printf("usb state[%d], wifi state[%d], lcd state[%d], "
                "voice call[%d], call active[%d]\n",
                cur_usb_state, cur_wifi_state, cur_lcd_state,
                voiceCallFlag, voiceCallActive);

        printf("autoSleepFlag[%d], lcdOff_time[%d], suspend_time[%d], "
                "gotolcdoff_time[%d], gotosleep_time[%d]\n", autoSleepFlag,
                lcdOff_time, suspend_time, gotolcdoff_time, gotosleep_time);
#endif

        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "%d,%d,%d,%d,%d,%d,%d,%d.%d,%d",
                cur_usb_state, cur_wifi_state, cur_lcd_state,
                voiceCallFlag, voiceCallActive,
                autoSleepFlag, lcdOff_time, suspend_time,
                gotolcdoff_time, gotosleep_time);
        property_set("sys.power.status", buf);

        if (cur_lcd_state == 1) {
            if (times++ > 20 || cur_brightness != def_brightness) {
                cur_wifi_state = ql_get_wifi_status();
                if (cur_wifi_state < 0) {
                    QL_WIFI_init();
                    //cur_wifi_state = 0;
                }
                ret = QLSCREEN_GetBrightness(&cur_brightness);
                times = 0;
            }

            /* 同步配置文件, 如果文件有更新就会同步参数 */
            ret = QL_Sync_Power_Manager_Conf();
            if (def_brightness < 10)
                def_brightness = 10;

            if (ret > 0 && cur_brightness > 0 && cur_brightness != def_brightness) {
                ret = QLSCREEN_SetBrightness(def_brightness);
                if (ret < 0)
                    printf("QLSCREEN_SetBrightness failure return:%d\n", ret);
            }

            if (low_power_mode > 0) {
                if (h_nw != 0) {
                    ret = QL_MCM_NW_SetLowPowerMode(h_nw, LOW_POWER_MODE_OFF);
                    printf("QL_MCM_NW_SetLowPowerMode off: %d\n", ret);
                }

                if (qlril_handle != 0) {
                    ret = QLRIL_SetScreenState(&qlril_handle, 1);
                    printf("QLRIL_SetScreenState on return: %d\n", ret);
					if (ret != 0)
						qlril_handle = 0;
                }

                low_power_mode = 0;
            }

            sleep(3);
        } else {
            if (low_power_mode <= 0) {
                if (h_nw != 0) {
                    ret = QL_MCM_NW_SetLowPowerMode(h_nw, LOW_POWER_MODE_ON);
                    printf("QL_MCM_NW_SetLowPowerMode on: %d\n", ret);
                }

                if (qlril_handle != 0) {
                    ret = QLRIL_SetScreenState(&qlril_handle, 0);
                    printf("QLRIL_SetScreenState off return: %d\n", ret);
                }

                low_power_mode = 1;
            }

            pthread_mutex_lock(&s_mutex);
            if (cur_wifi_state <= 0 && cur_usb_state == 0) {
                pthread_mutex_unlock(&s_mutex);

                if (gotosleep_time-- <= 0) {
                    printf("***System Sleep***\n");
                    gotosleep_time = 0;
                    usleep(10);
                    ret = QLMISC_EnterHibernate();
                    if (ret < 0)
                        printf("QLMISC_EnterHibernate failure:%d\n", ret);
                    else
                        printf("***System wakeup***\n");
                }
                sleep(1);
            } else {
                pthread_mutex_unlock(&s_mutex);
                sleep(2);
            }
        }
    }
}

static int switch_lcd_power(void)
{
    int ret = 0;
    int keyfd = 0;
    int tsfd = 0;
    int change = 0;
    int pipefds[2];
    int max_fd = 0;
    int usb_oldstate = 1;
    int i, count;
    int first_flag = 1;
    char buf[1024];
    struct timeval tv;
    struct timeval old_tv;
    struct timeval new_tv;
    struct input_event value[EVENT_NUM];
    fd_set read_fds;

    usb_oldstate = cur_usb_state;
    if (cur_usb_state == 1)
        property_set("sys.usb.connect", "1");
    else
        property_set("sys.usb.connect", "0");

    gettimeofday(&old_tv, NULL);

    for (;;) {
		max_fd = 0;
        change = 0;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        FD_ZERO(&read_fds);

        if (keyfd <= 0) {
            keyfd = open(KEY_PWK_EVENT_FILE_NAME, O_RDONLY);
            if (keyfd < 0) {
                perror("open event failed");
                keyfd = 0;
            }
        }

		if (keyfd > 0){
            FD_SET(keyfd, &read_fds);
			if (max_fd < keyfd)
				max_fd = keyfd;
		}

        if (s_pipeReadFd == 0 || s_pipeWriteFd == 0) {
            ret = pipe(pipefds);
            if (ret < 0) {
                perror("pipe failed");
                s_pipeReadFd = 0;
            } else {
                s_pipeReadFd = pipefds[0];
                s_pipeWriteFd = pipefds[1];

                fcntl(s_pipeReadFd, F_SETFL, O_NONBLOCK);
            }
        }

		if (s_pipeReadFd > 0){
            FD_SET(s_pipeReadFd, &read_fds);
            if (max_fd < s_pipeReadFd)
                max_fd = s_pipeReadFd;
        }

        if (autoSleepFlag == 1 && first_flag == 0) {
            if (tsfd <= 0) {
                tsfd = open("/dev/input/event1",02);
                if (tsfd < 0) {
                    perror("open event1 failed");
                    tsfd = 0;
                } else {
                    //fcntl(tsfd, F_SETFL, O_NONBLOCK);
                    //FD_SET(tsfd, &read_fds);
                }
            }

			if (tsfd > 0){
                FD_SET(tsfd, &read_fds);
                if (max_fd < tsfd)
                    max_fd = tsfd;
			}

#if 0
            if (usbfd <= 0) {
                usbfd = open(ANDROID_USB_STATE_FILE_NAME, O_RDONLY);
                if (usbfd < 0) {
                    perror("open usb state file failed");
                    usbfd = 0;
                } else {
                    if (max_fd < usbfd)
                        max_fd = usbfd;
                    FD_SET(usbfd, &read_fds);
                }
            } else {
                FD_SET(usbfd, &read_fds);
            }
#endif
        }

        ret = select(max_fd + 1, &read_fds, NULL, NULL, &tv);
        if (ret <= 0) {
            if (ret == 0) {
                if (autoSleepFlag == 1 && cur_lcd_state == 1) {
                    if (--gotolcdoff_time <= 0) {
                        if (first_flag == 1) {
                            gettimeofday(&new_tv, NULL);
                            if ((new_tv.tv_sec - old_tv.tv_sec) >= lcdOff_time) {
                                first_flag = 0;
                                usb_oldstate = cur_lcd_state;
                                change = 3;
                            }
                        } else
                            change = 3;
                    } else
                        continue;
                } else if (cur_lcd_state == 0 && usb_oldstate != cur_usb_state) {
                    usb_oldstate = cur_usb_state;
                    if (cur_usb_state == 1) {
                        change = 2;
                        property_set("sys.usb.connect", "1");
                    } else {
                        property_set("sys.usb.connect", "0");
                        continue;
                    }
                } else {
                    errno = ETIMEDOUT;
                    continue;
                }
            } else {
                perror ("Big error select");
				if (keyfd != 0)
					close(keyfd);

				if (tsfd != 0)
					close(tsfd);

				if (s_pipeReadFd != 0)
					close(s_pipeReadFd);

				if (s_pipeWriteFd != 0)
					close(s_pipeWriteFd);

                keyfd = 0;
                tsfd = 0;
                s_pipeReadFd = 0;
                s_pipeWriteFd = 0;
                continue;
            }
        }

        if (FD_ISSET(tsfd, &read_fds)) {
            ret = read(tsfd, buf, sizeof(buf));
            gotolcdoff_time = lcdOff_time;
            gotosleep_time = suspend_time;
            change = 0;
        }

        if (FD_ISSET(keyfd, &read_fds)) {
            count = read(keyfd, value, EVENT_NUM * sizeof(struct input_event));
            if (count < 0) {
                fprintf(stderr, "read data from event failed\n");
                close(keyfd);
                keyfd = 0;
                sleep(1);
                continue;
            }

            count /= sizeof(struct input_event);
            for (i = 0; i < count; i++) {
                //printf("c:%d code:%d v:%d\n", count,value[i].code,value[i].value);
                if (value[i].value) {
                    if (value[i].code == KEY_PWK)
                        change = 1;
                }
            }
            //printf ("fulinux key pressed\n");
        }

        if (FD_ISSET(s_pipeReadFd, &read_fds)) {
            memset(buf, 0, sizeof(buf));
            ret = read(s_pipeReadFd, buf, sizeof(buf));
            //printf ("fulinux pipe recieve data buf:%s\n", buf);
            if (ret < 0) {
                close(s_pipeReadFd);
                close(s_pipeWriteFd);
                s_pipeReadFd = 0;
                s_pipeWriteFd = 0;
                change = 0;
            } else {
                if (change == 0) {
                    if (strstr(buf, "ON") != NULL) {
                        change = 4;
                    } else if (strstr(buf, "OFF") != NULL) {
                        change = 3;
                    } else if (strstr(buf, "RST") != NULL) {
                        change = 2;
                    }
                }
            }
        }

        if (change == 0) {
            continue;
		}

        //memset(buf, 0, sizeof(buf));
        //snprintf(buf, 2, "%d", change);
        //property_set("sys.power.change", buf);
        //printf ("[%s:%d] change = %d\n", __func__, __LINE__, change);

        pthread_mutex_lock(&s_mutex);
        if (change == 1) {
            if (cur_lcd_state == 1) {
                ret = QLSCREEN_PowerOff(screen_handle);
                if (ret == 0) {
                    cur_lcd_state = 0;
                    first_flag = 0;
                    gotosleep_time = suspend_time;
                }
            } else {
                ret = QLSCREEN_PowerOn(screen_handle);
                if (ret == 0) {
                    cur_lcd_state = 1;
                    gotolcdoff_time = lcdOff_time;
                }
            }
        } else if (change == 2) {
            if (cur_lcd_state == 0) {
                ret = QLSCREEN_PowerOn(screen_handle);
                if (ret == 0) {
                    cur_lcd_state = 1;
                    gotolcdoff_time = lcdOff_time;
                }
            } else {
                gotolcdoff_time += 5;
            }
        } else if (change == 3) {
            if (cur_lcd_state == 1) {
                ret = QLSCREEN_PowerOff(screen_handle);
                if (ret == 0) {
                    cur_lcd_state = 0;
                    first_flag = 0;
                    gotosleep_time = suspend_time;
                }
            }
        } else if (change == 4) {
            if (cur_lcd_state == 0) {
                ret = QLSCREEN_PowerOn(screen_handle);
                if (ret == 0) {
                    cur_lcd_state = 1;
                }
            }

            gotolcdoff_time = lcdOff_time;
        }
        pthread_mutex_unlock(&s_mutex);

        if (cur_lcd_state == 1) {//FIXME
            QLSCREEN_SetBrightness(cur_brightness);
        }
    }

    return 0;
}

int main (int argc, char **argv)
{
    int ret;
    pthread_t handle_tid;
    pthread_t sleep_tid;

    ret = QLSCREEN_Init(&screen_handle);
    if (ret < 0 || screen_handle == 0) {
        printf("QLSCREEN_Init failure with return:%d\n", ret);
        screen_handle = 0;
    }

    ret = QLSCREEN_PowerOn(screen_handle);
    if (ret == 0) {
        cur_lcd_state = 1;
    }

    ret = QLMISC_IsffbmMode();
    if (ret == 1) {
        ffbmModeFlag = 1;
        printf("ffbm Mode is true\n");
    } else {
        ffbmModeFlag = 0;

        cur_usb_state = QLMISC_USBCableState();
        ret = QL_Sync_Power_Manager_Conf();
        if (ret < 0) {
            printf("Don't use %s\n", POWER_MANAGER_CONFIG_FILE);
        }

        gotolcdoff_time = lcdOff_time;
        gotosleep_time = suspend_time;

        QLSCREEN_SetBrightness(def_brightness);

        ret = QL_WIFI_init();
        if (ret != 0) {
            printf("QL_WIFI_init error\n");
        } else
            printf("QL_WIFI_init success\n");

        ret = pthread_create(&sleep_tid, NULL, handle_system_sleep_thread, NULL);
        if (ret != 0) {
            perror("pthread_create handle_system_sleep_thread error");
        }
    }

    ret = switch_lcd_power();//block

    return 0;
}
