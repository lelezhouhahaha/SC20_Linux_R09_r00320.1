#ifndef _QCMAP_BT_H_
#define _QCMAP_BT_H_

/*====================================================

FILE:  QCMAP_BT_Tethering.h

SERVICES:
   QCMAP Connection Manager BT Tethering Class

=====================================================

  Copyright (c) 2016 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

=====================================================*/
/*======================================================
  EDIT HISTORY FOR MODULE

  Please notice that the changes are listed in reverse chronological order.
    when       who        what, where, why
  --------   ---        -------------------------------------------------------
  06/13/16   c_surajj           Created
======================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/select.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include "stringl.h"
#include "ds_util.h"
#include "comdef.h"
#include "qcmap_cm_api.h"

#include "QCMAP_Backhaul_WWAN.h"
#include "QCMAP_Backhaul_WLAN.h"
#include "QCMAP_Backhaul_Cradle.h"

#define BT_IFACE "bt-pan"

class QCMAP_BT_TETHERING
{
  private:
    static bool flag;
    static QCMAP_BT_TETHERING *object;
    QCMAP_BT_TETHERING();


  public:
    static QCMAP_BT_TETHERING* Get_Instance(boolean obj_create=false);

    ~QCMAP_BT_TETHERING();

    /*----------------------MobileAP BT Tethering Execution-----------------------*/
    qcmap_msgr_bt_tethering_status_enum_v01 bt_tethering_status;
    qcmap_bt_tethering_mode_enum_v01 bt_tethering_mode;

    /*V4 connection available over BT WAN*/
    boolean bt_v4_available;
    /*V6 connection available over BT WAN*/
    boolean bt_v6_available;
    /* Track BT WAN backahul bringup */
    boolean bt_connect_in_progress;

    /* Use to store the gateway information from RA when acting as backhaul*/
    struct in6_addr ipv6_gateway_addr;

    static boolean BringupBTTethering(qcmap_bt_tethering_mode_enum_v01
                   bt_tethering_mode, qmi_error_type_v01 *qmi_err_num);
    static boolean BringdownBTTethering( qmi_error_type_v01 *qmi_err_num);
    static boolean GetBTTetheringStatus
    (
      qcmap_msgr_bt_tethering_status_enum_v01 *bt_teth_status,
      qcmap_bt_tethering_mode_enum_v01 *bt_teth_mode,
      qmi_error_type_v01 *qmi_err_num
    );

    static void ProcessBTWANAddrAssign(char* devname,
		        ip_version_enum_type ip_vsn);
    void ProcessBTWANDisAssoc();
    void ProcAddrDisAssocOnBTWAN();
    static boolean IsBTBackhaulAvailableV4();
    static boolean IsBTBackhaulAvailableV6();
    void SwitchToBTBackhaul();

};
#endif

