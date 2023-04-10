#ifndef OBJECT_ACCESS_CONTROL_H
#define OBJECT_ACCESS_CONTROL_H

#define ACC_CTRL_PERM_CREATE    0x10    // 00010000
#define ACC_CTRL_PERM_DELETE    0x08    // 00001000
#define ACC_CTRL_PERM_EXECUTE   0x04    // 00000100
#define ACC_CTRL_PERM_WRITE     0x02    // 00000010
#define ACC_CTRL_PERM_READ      0x01    // 00000001
#define ACC_CTRL_PERM_NULL      0x00    // 00000000

#define ACC_CTRL_PERM_RWX       ACC_CTRL_PERM_EXECUTE | ACC_CTRL_PERM_WRITE | ACC_CTRL_PERM_READ

#define ACC_CTRL_CAN_READ(perm)     (perm & ACC_CTRL_PERM_READ)
#define ACC_CTRL_CAN_WRITE(perm)    (perm & ACC_CTRL_PERM_WRITE)
#define ACC_CTRL_CAN_EXEC(perm)     (perm & ACC_CTRL_PERM_EXECUTE)
#define ACC_CTRL_CAN_DELETE(perm)   (perm & ACC_CTRL_PERM_DELETE)
#define ACC_CTRL_CAN_CREATE(perm)   (perm & ACC_CTRL_PERM_CREATE)



typedef struct acc_ctrl_ri_s
{   // linked list:
  struct acc_ctrl_ri_s*   next;       // matches lwm2m_list_t::next
  uint16_t                resInstId;  // matches lwm2m_list_t::id, ..serverID
  // resource data:
  uint16_t                accCtrlValue;
} acc_ctrl_ri_t;

typedef struct acc_ctrl_oi_s
{   //linked list:
  struct acc_ctrl_oi_s*   next;       // matches lwm2m_list_t::next
  uint16_t                objInstId;  // matches lwm2m_list_t::id
  // resources
  uint16_t                objectId;
  uint16_t                objectInstId;
  uint16_t                accCtrlOwner;
  acc_ctrl_ri_t*          accCtrlValList;
} acc_ctrl_oi_t;


lwm2m_object_t* acc_ctrl_create_object(void);
void acl_ctrl_free_object(lwm2m_object_t * objectP);

#ifdef AUTHORIZATION_SUPPORT

acc_ctrl_oi_t* get_acc_cl_objectinst(lwm2m_context_t *contextP, lwm2m_uri_t *uriP);
acc_ctrl_ri_t* get_acc_cl_resourceinst(lwm2m_context_t *contextP, acc_ctrl_oi_t *acc_ctrl_oi);

#endif
int load_ac_persistent_info(lwm2m_object_t *aclObjP);


#endif
