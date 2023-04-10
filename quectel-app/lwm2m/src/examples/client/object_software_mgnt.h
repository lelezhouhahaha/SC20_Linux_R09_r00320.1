#ifndef OBJECT_SOFTWARE_MGNT_H
#define OBJECT_SOFTWARE_MGNT_H

#ifdef ENABLE_SOFTWARE_MGNT_OBJ

lwm2m_object_t * get_object_software_mgnt();

void free_object_softwaremgnt(lwm2m_object_t * objectP);

#endif

#endif

