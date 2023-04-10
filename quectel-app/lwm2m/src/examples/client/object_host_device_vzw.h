#ifndef OBJECT_HOST_DEVICE_VZW_H
#define OBJECT_HOST_DEVICE_VZW_H

// ---- private "object host device" specific defines ----
// Resource Id's:
#define RES_M_HOST_DEV_MANUFACTURER     0
#define RES_M_HOST_DEV_MODEL            1
#define RES_M_HOST_DEV_ID               2
#define RES_M_HOST_DEV_FW_VER           3
#define RES_M_HOST_DEV_SW_VER           4
#define RES_M_HOST_DEV_HW_VER           5
#define RES_M_HOST_DEV_UPDATE_TM        6

#define TEMP_LWM2M_MALLOC_SIZE          30   //Temporory Array size
#define PRV_EMPTY                       " "
typedef struct object_host_dev_
{
  struct object_host_dev_ *next;
  uint16_t instanceId;
  uint8_t host_dev_manufacturer[128];
  uint8_t host_dev_model[128];
  uint8_t host_dev_id[128];
  char    sw_ver[128];
  char    fw_ver[128];
  char    hw_ver[128];
  int64_t upgrade_time;
}object_host_dev_t;

#endif

