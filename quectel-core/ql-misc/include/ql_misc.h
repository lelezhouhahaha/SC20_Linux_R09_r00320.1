#ifndef _QL_MISC_H_
#define _QL_MISC_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MISC_SYS_PROP_SYSTEM_SLEEP_NOW "ctrl.sleep.now"

int QL_MISC_Reboot_Modem(void);
int QLMISC_RebootModem(void);

int QL_MISC_System_Sleep(void);
int QLMISC_SystemSleep(void);
int QLMISC_EnterHibernate(void);

int QL_MISC_SendAT(char *cmd, char *resp, size_t size);
int QLMISC_SendAT(char *cmd, char *resp, size_t size);
int QLMISC_SendAtByDev(char *device, char *cmd, char *resp, size_t size);

int QLSCREEN_Init(int *handle);
int QLSCREEN_Exit(int *handle);
int QLSCREEN_PowerOn(int handle);
int QLSCREEN_PowerOff(int handle);

int QLSCREEN_SetBrightness(unsigned char value);
int QLSCREEN_GetBrightness(unsigned char *value);

int QLMISC_USBCableState(void);

int QLMISC_IsffbmMode(void);
int QLMISC_IsRunning(char *process);

#ifdef __cplusplus
}
#endif

#endif
