/*******************************************************************************
 *
 * Copyright (c) 2015 Intel Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v20.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    David Navarro, Intel Corporation - initial API and implementation
 *    Christian Renz - Please refer to git log
 *
 *******************************************************************************/

#define LOG_TAG "Mango-dtlsconnection.c"
#include "lwm2m_android_log.h"
#include "call_stack.h"
#include <cutils/properties.h>
#include <unistd.h>

#define MAX_SIZE 125
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dtlsconnection.h"
#include "commandline.h"
#include "platform.h"
#include "internals.h"
#define MBEDTLS_PSK_MAX_LEN 32

#define COAP_PORT "5683"
#define COAPS_PORT "5684"
#define URI_LENGTH 256

bool write_enable = false;

/********************* Security Obj Helpers **********************/
char * security_get_uri(lwm2m_object_t * obj, int instanceId, char * uriBuffer, int bufferSize){
    int size = 1;
    lwm2m_data_t * dataP = lwm2m_data_new(size);
    dataP->id = 0; // security server uri

    obj->readFunc(instanceId, &size, &dataP, obj);
    if (dataP != NULL &&
            dataP->type == LWM2M_TYPE_STRING &&
            dataP->value.asBuffer.length > 0)
    {
        if (bufferSize > dataP->value.asBuffer.length){
            memset(uriBuffer,0,dataP->value.asBuffer.length+1);
            strncpy(uriBuffer,(const char *)dataP->value.asBuffer.buffer,dataP->value.asBuffer.length);
            lwm2m_data_free(size, dataP);
            return uriBuffer;
        }
    }
    lwm2m_data_free(size, dataP);
    return NULL;
}

int64_t security_get_mode(lwm2m_object_t * obj, int instanceId){
    int64_t mode;
    int size = 1;
    lwm2m_data_t * dataP = lwm2m_data_new(size);
    dataP->id = 2; // security mode

    obj->readFunc(instanceId, &size, &dataP, obj);
    if (0 != lwm2m_data_decode_int(dataP,&mode))
    {
        lwm2m_data_free(size, dataP);
        return mode;
    }

    lwm2m_data_free(size, dataP);
    fprintf(stderr, "Unable to get security mode : use not secure mode");
    return LWM2M_SECURITY_MODE_NONE;
}

char * security_get_public_id(lwm2m_object_t * obj, int instanceId, int * length){
    int size = 1;
    lwm2m_data_t * dataP = lwm2m_data_new(size);
    dataP->id = 3; // public key or id

    obj->readFunc(instanceId, &size, &dataP, obj);
    if (dataP != NULL &&
        dataP->type == LWM2M_TYPE_OPAQUE)
    {
        char * buff;

        buff = (char*)lwm2m_malloc(dataP->value.asBuffer.length);
        if (buff != 0)
        {
            memcpy(buff, dataP->value.asBuffer.buffer, dataP->value.asBuffer.length);
            *length = dataP->value.asBuffer.length;
        }
        lwm2m_data_free(size, dataP);

        return buff;
    } else {
        return NULL;
    }
}

//add by joe start
bool get_bootstrap_id_for_vzw(lwm2m_object_t * obj, int instanceId){
    int size = 1;
    lwm2m_data_t * dataP = lwm2m_data_new(size);
    dataP->id = LWM2M_SECURITY_BOOTSTRAP_ID; // get bootstrap id
	bool isBootstrap = true;
    LOG_ARG("get_bootstrap_id_for_vzw isBootstrap=%d\n",isBootstrap);
    obj->readFunc(instanceId, &size, &dataP, obj);
	//klein: get the value of isBootstrap
    if (dataP != NULL &&dataP->type == LWM2M_TYPE_BOOLEAN)
	{
        LOG("get_bootstrap_id_for_vzw call lwm2m_data_decode_bool\n");
        if (0 == lwm2m_data_decode_bool(dataP, &isBootstrap))
        {
            lwm2m_data_free(size, dataP);
            return -1;
        }
	}
    LOG_ARG("get_bootstrap_id_for_vzw isBootstrap=%d\n",isBootstrap);
    lwm2m_data_free(size, dataP);
    return isBootstrap;
}
//add by joe end

bool get_bootstrap_id(lwm2m_context_t * contextP){
    lwm2m_object_t * objectP;
    lwm2m_object_t * securityObjP = NULL;
    lwm2m_object_t * serverObjP = NULL;
    lwm2m_list_t * securityInstP;   // instanceID of the server in the LWM2M Security Object

    LOG("get_bootstrap_id");
	bool isBootstrap;

    for (objectP = contextP->objectList; objectP != NULL; objectP = objectP->next)
    {
        if (objectP->objID == LWM2M_SECURITY_OBJECT_ID)
        {
            securityObjP = objectP;
        }
    }

    if (NULL == securityObjP) return -1;

    securityInstP = securityObjP->instanceList;
	lwm2m_data_t * dataP;
	int size;
	lwm2m_server_t * targetP;
	int64_t value = 0;
	
	size = 1;
	dataP = lwm2m_data_new(size);
	if (dataP == NULL) return -1;
	dataP[0].id = LWM2M_SECURITY_BOOTSTRAP_ID;
	//klein: get the value of isBootstrap
	if (securityObjP->readFunc(securityInstP->id, &size, &dataP, securityObjP) != COAP_205_CONTENT)
	{
		lwm2m_data_free(size, dataP);
		return -1;
	}

	LOG_ARG("dataP[0].type =  %d, dataP[0].id = %d, dataP[0].value.asBoolean = %d", dataP[0].type, dataP[0].id, dataP[0].value.asBoolean);

	if (0 == lwm2m_data_decode_bool(dataP, &isBootstrap))
	{
		lwm2m_free(targetP);
		lwm2m_data_free(size, dataP);
		return -1;
	}

	 return isBootstrap;
}

char * security_get_secret_key(lwm2m_object_t * obj, int instanceId, int * length){
    int size = 1;
    lwm2m_data_t * dataP = lwm2m_data_new(size);
    dataP->id = 5; // secret key

    obj->readFunc(instanceId, &size, &dataP, obj);
    if (dataP != NULL &&
        dataP->type == LWM2M_TYPE_OPAQUE)
    {
        char * buff;

        buff = (char*)lwm2m_malloc(dataP->value.asBuffer.length);
        if (buff != 0)
        {
            memcpy(buff, dataP->value.asBuffer.buffer, dataP->value.asBuffer.length);
            *length = dataP->value.asBuffer.length;
        }
        lwm2m_data_free(size, dataP);

        return buff;
    } else {
        return NULL;
    }
}

/********************* Security Obj Helpers Ends **********************/


int get_port(struct sockaddr *x)
{
   if (x->sa_family == AF_INET)
   {
       return ((struct sockaddr_in *)x)->sin_port;
   } else if (x->sa_family == AF_INET6) {
       return ((struct sockaddr_in6 *)x)->sin6_port;
   } else {
       LOG("non IPV4 or IPV6 address\n");
       return  -1;
   }
}

int sockaddr_cmp(struct sockaddr *x, struct sockaddr *y)
{
    int portX = get_port(x);
    int portY = get_port(y);

    // if the port is invalid of different
    if (portX == -1 || portX != portY)
    {
        return 0;
    }

    // IPV4?
    if (x->sa_family == AF_INET)
    {
        // is V4?
        if (y->sa_family == AF_INET)
        {
            // compare V4 with V4
            return ((struct sockaddr_in *)x)->sin_addr.s_addr == ((struct sockaddr_in *)y)->sin_addr.s_addr;
            // is V6 mapped V4?
        } else if (IN6_IS_ADDR_V4MAPPED(&((struct sockaddr_in6 *)y)->sin6_addr)) {
            struct in6_addr* addr6 = &((struct sockaddr_in6 *)y)->sin6_addr;
            uint32_t y6to4 = addr6->s6_addr[15] << 24 | addr6->s6_addr[14] << 16 | addr6->s6_addr[13] << 8 | addr6->s6_addr[12];
            return y6to4 == ((struct sockaddr_in *)x)->sin_addr.s_addr;
        } else {
            return 0;
        }
    } else if (x->sa_family == AF_INET6 && y->sa_family == AF_INET6) {
        // IPV6 with IPV6 compare
        return memcmp(((struct sockaddr_in6 *)x)->sin6_addr.s6_addr, ((struct sockaddr_in6 *)y)->sin6_addr.s6_addr, 16) == 0;
    } else {
        // unknown address type
        LOG("non IPV4 or IPV6 address\n");
        return 0;
    }
}

int create_socket(const char * portStr, int ai_family)
{
    int s = -1;
    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = ai_family;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if (0 != getaddrinfo(NULL, portStr, &hints, &res))
    {
        return -1;
    }

    for(p = res ; p != NULL && s == -1 ; p = p->ai_next)
    {
        s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (s >= 0)
        {
            if (-1 == bind(s, p->ai_addr, p->ai_addrlen))
            {
                close(s);
                s = -1;
            }
        }
    }

    freeaddrinfo(res);

    return s;
}

dtls_connection_t * connection_find(dtls_connection_t * connList,
                               const struct sockaddr_storage * addr,
                               size_t addrLen)
{
    dtls_connection_t * connP;

    connP = connList;
    while (connP != NULL)
    {

       if (sockaddr_cmp((struct sockaddr*) (&connP->addr),(struct sockaddr*) addr)) {
            return connP;
       }

       connP = connP->next;
    }

    return connP;
}

#ifdef WITH_TINYDTLS

dtls_context_t * dtlsContext;


/* Returns the number sent, or -1 for errors */
int send_data(dtls_connection_t *connP,
                    uint8_t * buffer,
                    size_t length)
{
    LOG_ARG("send_data, length = %d", length);
    //dump_stack("mango-send_data");
    int nbSent;
    size_t offset;

#ifdef WITH_LOGS
    char s[INET6_ADDRSTRLEN];
    in_port_t port;

    s[0] = 0;

    if (AF_INET == connP->addr.sin6_family)
    {
        struct sockaddr_in *saddr = (struct sockaddr_in *)&connP->addr;
        inet_ntop(saddr->sin_family, &saddr->sin_addr, s, INET6_ADDRSTRLEN);
        port = saddr->sin_port;
    }
    else if (AF_INET6 == connP->addr.sin6_family)
    {
        struct sockaddr_in6 *saddr = (struct sockaddr_in6 *)&connP->addr;
        inet_ntop(saddr->sin6_family, &saddr->sin6_addr, s, INET6_ADDRSTRLEN);
        port = saddr->sin6_port;
    }

    fprintf(stderr, "Sending %d bytes to [%s]:%hu\r\n", (int)length, s, ntohs(port));
	LOGD(stderr, "Sending %d bytes to [%s]:%hu\r\n", (int)length, s, ntohs(port));
    output_buffer(stderr, buffer, length, 0);
#endif

    offset = 0;
    while (offset != length)
    {
        nbSent = sendto(connP->sock, buffer + offset, length - offset, 0, (struct sockaddr *)&(connP->addr), connP->addrLen);
        if (nbSent == -1) return -1;
        offset += nbSent;
    }
    connP->lastSend = lwm2m_gettime();
    return offset;
}

/**************************  TinyDTLS Callbacks  ************************/


/* The callback function must return the number of bytes
 * that were sent, or a value less than zero to indicate an
 * error. */
static int send_to_peer(struct dtls_context_t *ctx,
        session_t *session, uint8 *data, size_t len) {

    // find connection
    dtls_connection_t* cnx = connection_find((dtls_connection_t *) ctx->app, &(session->addr.st),session->size);
    if (cnx != NULL)
    {
        // send data to peer

        // TODO: nat expiration?
        int res = send_data(cnx,data,len);
        if (res < 0)
        {
            return -1;
        }
        return res;
    }
    return -1;
}

static int read_from_peer(struct dtls_context_t *ctx,
          session_t *session, uint8 *data, size_t len) {
    LOG("read_from_peer X");
	output_buffer(stderr, data, len, 0);
	LOG_ARG("data = %s, len = %d", data, len);

    // find connection
    dtls_connection_t* cnx = connection_find((dtls_connection_t *) ctx->app, &(session->addr.st),session->size);
    if (cnx != NULL)
    {
        lwm2m_handle_packet(cnx->lwm2mH, (uint8_t*)data, len, (void*)cnx);
        return 0;
    }
    LOG("read_from_peer E");
    return -1;
}


/* This function is the "key store" for tinyDTLS. It is called to
 * retrieve a key for the given identity within this particular
 * session. */
static int get_psk_info(struct dtls_context_t *ctx,
        const session_t *session,
        dtls_credentials_type_t type,
        const unsigned char *id, size_t id_len,
        unsigned char *result, size_t result_length) {

    LOG("get_psk_info");
    //dump_stack("mango");
    // find connection
    dtls_connection_t* cnx = connection_find((dtls_connection_t *) ctx->app, &(session->addr.st),session->size);
    if (cnx == NULL)
    {
        LOG("GET PSK session not found\n");
        return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
    }
    LOG_ARG("get_psk_info, type = %d", type);

    switch (type) {
        case DTLS_PSK_IDENTITY:
        {
            int idLen;
            char * id;
            id = security_get_public_id(cnx->securityObj, cnx->securityInstId, &idLen);
            if (result_length < idLen)
            {
                LOG("cannot set psk_identity -- buffer too small\n");
                return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
            }

            LOG_ARG("id = %s, idLen = %d", id, idLen);

            memcpy(result, id,idLen);
            lwm2m_free(id);
            return idLen;
        }
        case DTLS_PSK_KEY:
        {
			LOG("DTLS_PSK_KEY");
            int keyLen;
            char * key;
            LOG_ARG("cnx->securityInstId = %d", cnx->securityInstId);
            key = security_get_secret_key(cnx->securityObj, cnx->securityInstId, &keyLen);

			LOG_ARG("key = %s, keyLen = %d, result_length = %d", key, keyLen, result_length);
            if (result_length < keyLen)
            {
                LOG("klein-cannot set psk -- buffer too small\n");
                return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
            }

            memcpy(result, key,keyLen);
            lwm2m_free(key);
            return keyLen;
        }
        case DTLS_PSK_HINT:
        {
            // PSK_HINT is optional and can be empty.
            return 0;
        }
        default:
            LOG_ARG("unsupported request type: %d\n", type);
    }

    return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
}


/**************************   TinyDTLS Callbacks Ends ************************/

static dtls_handler_t cb = {
  .write = send_to_peer,
  .read  = read_from_peer,
  .event = NULL,
//#ifdef DTLS_PSK
  .get_psk_info = get_psk_info,
//#endif /* DTLS_PSK */
//#ifdef DTLS_ECC
//  .get_ecdsa_key = get_ecdsa_key,
//  .verify_ecdsa_key = verify_ecdsa_key
//#endif /* DTLS_ECC */
};

dtls_context_t * get_dtls_context(dtls_connection_t * connList) {
    if (dtlsContext == NULL) {
        dtls_init();
        dtlsContext = dtls_new_context(connList);
        if (dtlsContext == NULL)
            fprintf(stderr, "Failed to create the DTLS context\r\n");
        dtls_set_handler(dtlsContext, &cb);
    }else{
        dtlsContext->app = connList;
    }
    return dtlsContext;
}

dtls_connection_t * connection_new_incoming(dtls_connection_t * connList,
                                       int sock,
                                       const struct sockaddr * addr,
                                       size_t addrLen)
{
    LOG("connection_new_incoming X");
    dtls_connection_t * connP;

    connP = (dtls_connection_t *)malloc(sizeof(dtls_connection_t));
    if (connP != NULL)
    {
        memset(connP, 0, sizeof(dtls_connection_t));
        connP->sock = sock;
        memcpy(&(connP->addr), addr, addrLen);
        connP->addrLen = addrLen;
        connP->next = connList;

        connP->dtlsSession = (session_t *)malloc(sizeof(session_t));
        memset(connP->dtlsSession, 0, sizeof(session_t));
        connP->dtlsSession->addr.sin6 = connP->addr;
        connP->dtlsSession->size = connP->addrLen;
        connP->lastSend = lwm2m_gettime();
    }
    LOG("connection_new_incoming E");

    return connP;
}

dtls_connection_t * connection_create(dtls_connection_t * connList,
                                 int sock,
                                 lwm2m_object_t * securityObj,
                                 int instanceId,
                                 lwm2m_context_t * lwm2mH,
                                 int addressFamily)
{
    LOG("connection_create X");
    struct addrinfo hints;
    struct addrinfo *servinfo = NULL;
    struct addrinfo *p;
    int s;
    struct sockaddr *sa;
    socklen_t sl;
    dtls_connection_t * connP = NULL;
    char uriBuf[URI_LENGTH];
    char * uri;
    char * host;
    char * port;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = addressFamily;
    hints.ai_socktype = SOCK_DGRAM;

    uri = security_get_uri(securityObj, instanceId, uriBuf, URI_LENGTH);
    if (uri == NULL) return NULL;

    LOG_ARG("uri = %s", uri);

    // parse uri in the form "coaps://[host]:[port]"
    char * defaultport;
    if (0 == strncmp(uri, "coaps://", strlen("coaps://")))
    {
        host = uri+strlen("coaps://");
        defaultport = COAPS_PORT;
    }
    else if (0 == strncmp(uri, "coap://", strlen("coap://")))
    {
        host = uri+strlen("coap://");
        defaultport = COAP_PORT;
    }
    else
    {
        return NULL;
    }

    port = strrchr(host, ':');
    if (port == NULL)
    {
        port = defaultport;
    }
    else
    {
        // remove brackets
        if (host[0] == '[')
        {
            host++;
            if (*(port - 1) == ']')
            {
                *(port - 1) = 0;
            }
            else
            {
                return NULL;
            }
        }
        // split strings
        *port = 0;
        port++;
    }

    LOG_ARG("uri = %s, host = %s, port = %d",uri, host, port);

    if (0 != getaddrinfo(host, port, &hints, &servinfo) || servinfo == NULL) return NULL;

    // we test the various addresses
    s = -1;
    for(p = servinfo ; p != NULL && s == -1 ; p = p->ai_next)
    {
        s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (s >= 0)
        {
            sa = p->ai_addr;
            sl = p->ai_addrlen;
            if (-1 == connect(s, p->ai_addr, p->ai_addrlen))
            {
                close(s);
                s = -1;
            }
        }
    }
    if (s >= 0)
    {
        connP = connection_new_incoming(connList, sock, sa, sl);
        close(s);

        // do we need to start tinydtls?
        if (connP != NULL)
        {
            connP->securityObj = securityObj;
            connP->securityInstId = instanceId;
            connP->lwm2mH = lwm2mH;

            if (security_get_mode(connP->securityObj,connP->securityInstId)
                     != LWM2M_SECURITY_MODE_NONE)
            {
                connP->dtlsContext = get_dtls_context(connP);
            }
            else
            {
                // no dtls session
                connP->dtlsSession = NULL;
            }
        }
    }

    if (NULL != servinfo) free(servinfo);
    LOG("connection_create E");

    return connP;
}

int connection_send(dtls_connection_t *connP, uint8_t * buffer, size_t length){
    LOGD("connection_send");
	//dump_stack("mango");

    LOGD("connection_send buffer = %s, length = %d", buffer, length);
    if (connP->dtlsSession == NULL) {
        // no security
        if ( 0 != send_data(connP, buffer, length)) {
            return -1 ;
        }
    } else {
        if (DTLS_NAT_TIMEOUT > 0 && (lwm2m_gettime() - connP->lastSend) > DTLS_NAT_TIMEOUT)
        {
            // we need to rehandhake because our source IP/port probably changed for the server
			LOG("rehandhake because our source IP/port probably changed");
			if ( connection_rehandshake(connP, false) != 0 )
            {
                LOG("can't send due to rehandshake error\n");
                return -1;
            }
        }
        if (-1 == dtls_write(connP->dtlsContext, connP->dtlsSession, buffer, length)) {
            return -1;
        }
    }

    return 0;
}

int connection_handle_packet(dtls_connection_t *connP, uint8_t * buffer, size_t numBytes){
    LOG("connection_handle_packet X");
    LOG_ARG("connection_handle_packet, connP->dtlsSession = %p", connP->dtlsSession);

    if (connP->dtlsSession != NULL)
    {
        // Let liblwm2m respond to the query depending on the context
        int result = dtls_handle_message(connP->dtlsContext, connP->dtlsSession, buffer, numBytes);
        if (result !=0) {
             LOG_ARG("error dtls handling message %d\n",result);
        }
        return result;
    } else {
        // no security, just give the plaintext buffer to liblwm2m
        lwm2m_handle_packet(connP->lwm2mH, buffer, numBytes, (void*)connP);
        return 0;
    }
    LOG("connection_handle_packet E");
}

int connection_rehandshake(dtls_connection_t *connP, bool sendCloseNotify) {

    // if not a dtls connection we do nothing
    if (connP->dtlsSession == NULL) {
        return 0;
    }

    // reset current session
    dtls_peer_t * peer = dtls_get_peer(connP->dtlsContext, connP->dtlsSession);
    if (peer != NULL)
    {
        if (!sendCloseNotify)
        {
            peer->state =  DTLS_STATE_CLOSED;
        }
        dtls_reset_peer(connP->dtlsContext, peer);
    }

    // start a fresh handshake
    int result = dtls_connect(connP->dtlsContext, connP->dtlsSession);
    if (result !=0) {
         LOG_ARG("error dtls reconnection %d\n",result);
    }
    return result;
}

void connection_free(dtls_connection_t * connList)
{
    dtls_free_context(dtlsContext);
    dtlsContext = NULL;
    while (connList != NULL)
    {
        dtls_connection_t * nextP;

        nextP = connList->next;
        free(connList);

        connList = nextP;
    }
}

#else  //WITH_MBEDTLS

int get_ciphersuite_list()
{
    int index = 1;
    const int *list;
    const char *name;

    LOG("Available Ciphersuite:");
    list = mbedtls_ssl_list_ciphersuites();
    for(; *list; list++) {
        name = mbedtls_ssl_get_ciphersuite_name(*list);
        LOG_ARG("  [%03d] %s", index++, name);
    }

    return 0;
}

int save_connect_info(lwm2m_context_t *lwm2mH, char *host, char *port, char* psk_id, uint8_t *psk, int keyLen, mbedtls_ssl_session* saved_session)
{
	dm_server_connect_info * connect_info = (dm_server_connect_info *)malloc(sizeof(dm_server_connect_info));
	strcpy(connect_info->endpointname, lwm2mH->endpointName);
	strcpy(connect_info->serverAddr, host);
	strcpy(connect_info->serverPort, port);
	memcpy(connect_info->psk, psk, keyLen);
	connect_info->pskLen = keyLen;
	strcpy(connect_info->pskId, psk_id);
	connect_info->lifetime = lwm2mH->serverList->lifetime;
	connect_info->lastRegistrationTime = 0;
	
	LOG_ARG("saved_session->id = %s", saved_session->id);
	output_buffer(stderr, saved_session->id, 32, 0);
	
	memcpy(&(connect_info->saved_session), saved_session, sizeof(mbedtls_ssl_session));
	lwm2mH->connect_info = connect_info;
	LOG_ARG("sizeof(connect_info) = %d", sizeof(dm_server_connect_info));
	write_data(connect_info, sizeof(dm_server_connect_info));
	dump_connect_info();
    return 0;
}


//add by joe start
int save_connect_info_vzw(int instanceId,lwm2m_context_t *lwm2mH, char *host, char *port, char* psk_id, uint8_t *psk, int keyLen, mbedtls_ssl_session* saved_session)
{
	dm_server_connect_info * connect_info = (dm_server_connect_info *)malloc(sizeof(dm_server_connect_info));
    memset(connect_info, 0, sizeof(dm_server_connect_info));
	strcpy(connect_info->endpointname, lwm2mH->endpointName);
	strcpy(connect_info->serverAddr, host);
	strcpy(connect_info->serverPort, port);
	memcpy(connect_info->psk, psk, keyLen);
	connect_info->pskLen = keyLen;
	strcpy(connect_info->pskId, psk_id);
	connect_info->lifetime = lwm2mH->serverList->lifetime;
	connect_info->lastRegistrationTime = 0;
	//connect_info->location = {0};
	//LOG_ARG("saved_session->id = %s\n", saved_session->id);
	//output_buffer(stderr, saved_session->id, 32, 0);
	
	memcpy(&(connect_info->saved_session), saved_session, sizeof(mbedtls_ssl_session));
	lwm2mH->connect_info = connect_info;
	LOG_ARG("sizeof(connect_info) = %d\n", sizeof(dm_server_connect_info));
	write_data_for_vzw(instanceId,connect_info, sizeof(dm_server_connect_info));
	dump_connect_info_for_vzw(instanceId);
    //free(connect_info);
    //connect_info = NULL;
    return 0;
}

bool lwm2m_dtls_session_resumption(dtls_connection_t * connList, lwm2m_context_t * lwm2mH){
    LOG("lwm2m_dtls_session_resumption Start\n");

    //get server addr and port
    //-------------------------------------------------
    dtls_connection_t * connP = NULL;
    char uriBuf[URI_LENGTH];
    char * uri;
    char * host;
    char * port;
    uri = security_get_uri(connList->securityObj, connList->securityInstId, uriBuf, URI_LENGTH);

    if(connList->securityInstId == 3 && checkifApn3unAvailable()){ //add by joe for repo server
        //do not update
        return true;
       // LOG("do not create repo server");
    }
    if (uri == NULL) return NULL;

    LOG_ARG("uri = %s", uri);

    // parse uri in the form "coaps://[host]:[port]"
    char * defaultport;
    if (0 == strncmp(uri, "coaps://", strlen("coaps://")))
    {
        host = uri+strlen("coaps://");
        defaultport = COAPS_PORT;
    }
    else if (0 == strncmp(uri, "coap://", strlen("coap://")))
    {
        host = uri+strlen("coap://");
        defaultport = COAP_PORT;
    }
    else
    {
        return false;
    }

    port = strrchr(host, ':');
    if (port == NULL)
    {
        port = defaultport;
    }
    else
    {
        // remove brackets
        if (host[0] == '[')
        {
            host++;
            if (*(port - 1) == ']')
            {
                *(port - 1) = 0;
            }
            else
            {
                return false;
            }
        }
        // split strings
        *port = 0;
        port++;
    }

    LOG_ARG("uri = %s, host = %s, port = %s",uri, host, port);

    //get psk identy

    int psk_id_len;
    char * id;
    char psk_id[125]={0};
    id = security_get_public_id(connList->securityObj, connList->securityInstId, &psk_id_len);
    LOG_ARG("id = %s, psk_id_len = %d", id, psk_id_len);
    //memset(psk_id, 0,125);
    memcpy(psk_id, id,psk_id_len);
    LOG_ARG("after copy psk_id = %s",psk_id);
    //lwm2m_free(id);

    //get psk

    int keyLen;
    char * key;
    LOG_ARG("MBEDTLS_PSK_MAX_LEN = %d", MBEDTLS_PSK_MAX_LEN);
    //uint8_t psk[MBEDTLS_PSK_MAX_LEN];  //if set 32, and psk is 16Bytes, will stop in SSL/TLS handshake, why ?
    LOG_ARG("instanceId = %d", connList->securityInstId);
    key = security_get_secret_key(connList->securityObj, connList->securityInstId, &keyLen);

    LOG_ARG("key = %s, keyLen = %d", key, keyLen);
    uint8_t psk[keyLen];
    memcpy(psk, key,keyLen);
    LOG_ARG("after copy  key = %s", key);
    //lwm2m_free(key);
    //-------------------------------------------------------------

    int ret = 0, len = 0;
    unsigned char buf[256];
    get_ciphersuite_list();

    const char *pers = "dtls_client";

    mbedtls_entropy_context* entropy;
    mbedtls_ctr_drbg_context* ctr_drbg;
    mbedtls_ssl_context* ssl;
    mbedtls_ssl_config* conf;
    mbedtls_net_context* ctx;
    mbedtls_timing_delay_context* timer;

    mbedtls_ssl_session* saved_session;

    entropy = (mbedtls_entropy_context *)malloc(sizeof(mbedtls_entropy_context));
    ctr_drbg = (mbedtls_ctr_drbg_context *)malloc(sizeof(mbedtls_ctr_drbg_context));
    ssl = (mbedtls_ssl_context *)malloc(sizeof(mbedtls_ssl_context));
    conf = (mbedtls_ssl_config *)malloc(sizeof(mbedtls_ssl_config));
    ctx = (mbedtls_net_context *)malloc(sizeof(mbedtls_net_context));
    timer = (mbedtls_timing_delay_context *)malloc(sizeof(mbedtls_timing_delay_context));
	saved_session  = (mbedtls_ssl_session *)malloc(sizeof(mbedtls_ssl_session));
    memset(saved_session, 0, sizeof(mbedtls_ssl_session));

    mbedtls_net_init(ctx);
    mbedtls_ssl_init(ssl);
    mbedtls_ssl_config_init(conf);
    mbedtls_ctr_drbg_init(ctr_drbg);

    /*
     * 0. Initialize the RNG and the session data
     */
    LOG("\n	. Seeding the random number generator...");
    mbedtls_entropy_init(entropy);
    //mbedtls_entropy_add_source(&entropy, entropy_source, NULL,
    //MBEDTLS_ENTROPY_MAX_GATHER, MBEDTLS_ENTROPY_SOURCE_STRONG);
    ret = mbedtls_ctr_drbg_seed(ctr_drbg, mbedtls_entropy_func, entropy,
                                (const uint8_t *)pers, strlen(pers));

    /*
     * 2. Start the connection
     */
    LOG_ARG(" ok\n  . Connecting to %s:%s...", host, port);
    ret = mbedtls_net_connect(ctx, host, port, MBEDTLS_NET_PROTO_UDP );
    if( ret != 0 )
    {
        LOG_ARG( " failed\n  ! mbedtls_net_connect returned %d\n\n", ret );
        #ifdef ENABLE_LWM2M_NETWORK_MANAGERMENT
        //my need to check network status
        if(lwm2m_check_is_vzw_netwrok()){
            if(get_vzw_apn3_callid() == -2){
                quec_check_class2_apn_network_available();
                if(ret == MBEDTLS_ERR_NET_UNKNOWN_HOST){
                    LOG("\n . host resolv issue..\n");
                    configure_default_route(VZW_CLASS2_APN_IFNAME);
                }
            }else{
                quec_check_apn_network_available();
            }
        }
        #endif
    }


    ret = mbedtls_net_set_nonblock(ctx );
    if( ret != 0 )
    {
        LOG_ARG( " failed\n	! net_set_(non)block() returned -0x%x\n\n",
              -ret );
        #ifdef DTLS_SESSION_FREQUENTLY_RESUMPTION
        connList->last_conn_time = lwm2m_gettime();
        #endif
        connList->dlts_resumption = 0;//retry 1min later

        return false;//add by joe
    }

    /*
     * 3. Setup stuff
     */
    connList->dlts_resumption = 1;

    LOG(" ok\n  . Setting up the SSL/TLS structure...");
    ret = mbedtls_ssl_config_defaults(conf, MBEDTLS_SSL_IS_CLIENT,
                                      MBEDTLS_SSL_TRANSPORT_DATAGRAM, MBEDTLS_SSL_PRESET_DEFAULT);

    mbedtls_ssl_conf_rng(conf, mbedtls_ctr_drbg_random, ctr_drbg);
    ret = mbedtls_ssl_conf_psk(conf, psk, sizeof(psk),
                               (const uint8_t *)psk_id, psk_id_len);
    LOG_ARG("ret = %d", ret);
    if (ret != 0){
        LOG_ARG("set psk failed, ret = %x", ret);
    }
    mbedtls_ssl_set_timer_cb(ssl, timer, mbedtls_timing_set_delay, mbedtls_timing_get_delay);
    LOG("mbedtls_ssl_set_timer_cb\n");
    mbedtls_ssl_set_bio(ssl, ctx, mbedtls_net_send, mbedtls_net_recv, NULL );
    LOG("mbedtls_ssl_set_bio\n");
    ret = mbedtls_ssl_setup(ssl, conf);
    LOG_ARG("ret = %d\n", ret);
    mbedtls_ssl_conf_handshake_timeout(conf,5000, 10000);
    mbedtls_ssl_conf_read_timeout(conf, 0);
    //int cs = mbedtls_ssl_get_ciphersuite_id("TLS-PSK-WITH-AES-128-CBC-SHA256"); //mbedtls_ssl_get_ciphersuite_id("TLS-PSK-WITH-AES-128-CCM-8");

    //mbedtls_ssl_conf_ciphersuites( &conf, &cs);

    //use last time connect info
    
	dm_server_connect_info * connect_info = (dm_server_connect_info *)malloc(sizeof(dm_server_connect_info));
    memset(connect_info, 0, sizeof(dm_server_connect_info));
    int read_conn_info = 0;
    read_conn_info = read_data_for_vzw(connList->securityInstId,connect_info, sizeof(dm_server_connect_info));
    if(read_conn_info != 0){
     //   LOG("read_data_for_vzw 1.\n");
        return false;
    }
    
    //LOG_ARG("connect_info->saved_session = %s", connect_info->saved_session);
	//output_buffer(stderr, (dm_server_connect_info*)connect_info->saved_session.id, 32, 0);

    if( ( ret = mbedtls_ssl_session_reset(ssl) ) != 0 )
    {
         LOG_ARG( " failed\n	! mbedtls_ssl_session_reset returned -0x%x\n\n",
								-ret );
         #ifdef DTLS_SESSION_FREQUENTLY_RESUMPTION
         connList->last_conn_time = lwm2m_gettime();
         #endif
         connList->dlts_resumption = 0;//retry 1min later
         return false;//add by joe
    }

    if( ( ret = mbedtls_ssl_set_session(ssl, &(connect_info->saved_session))) != 0 )
    {
        LOG_ARG( " failed\n	! mbedtls_ssl_conf_session returned %d\n\n",
								ret );
        #ifdef DTLS_SESSION_FREQUENTLY_RESUMPTION
        connList->last_conn_time = lwm2m_gettime();
        #endif
        connList->dlts_resumption = 0;//retry 1min later
        return false;//add by joe
    }

	//memcpy( ssl->session_negotiate, saved_session, sizeof(mbedtls_ssl_session));

    while ((ret = mbedtls_ssl_handshake(ssl)) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            LOG_ARG(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret);
            #ifdef DTLS_SESSION_FREQUENTLY_RESUMPTION
            connList->last_conn_time = lwm2m_gettime();
            #endif
            connList->dlts_resumption = 0;//retry 1min later
            return false;//add by joe
        }
    }

   // LOG_ARG("reconnect sessionid = %s\n", ssl->session->id);
	//output_buffer(stderr, ssl->session->id, 32, 0);

    /*
     * 4. Handshake
     * 5. init dtls_connection_t
     */
    //#ifdef DTLS_SESSION_FREQUENTLY_RESUMPTION
    resumption_last_connection_free(connList);//free last connection
    //#endif
    LOG(" ok\n  . init dtls_connection_t...\n");
    connList->lastSend =  lwm2m_gettime();
    connList->last_conn_time = lwm2m_gettime();
    connList->dlts_resumption = 0;
    connList->net_ctx = ctx;
    connList->ssl_ctx = ssl;
    if(lwm2m_check_is_vzw_netwrok()){
        update_session_info_for_vzw(connList->securityInstId, lwm2mH, &(connect_info->saved_session));
    }else{ // for att
	    save_connect_info(lwm2mH, host, port, psk_id, psk, keyLen, &(connect_info->saved_session));
    }
    //free(connect_info);
    //connect_info = NULL;
	//write_data_for_vzw(instanceId,connect_info, sizeof(dm_server_connect_info));
    
    LOG("lwm2m_dtls_session_resumption success\n");
    return true;
}
//add by joe end
dtls_connection_t * connection_create(dtls_connection_t * connList,
                                      int sock,
                                      lwm2m_object_t * securityObj,
                                      int instanceId,
                                      lwm2m_context_t * lwm2mH,
                                      int addressFamily)
{
    LOG("connection_create X");

    //get server addr and port
    //-------------------------------------------------
    dtls_connection_t * connP = NULL;
    char uriBuf[URI_LENGTH];
    char * uri;
    char * host;
    char * port;
    uri = security_get_uri(securityObj, instanceId, uriBuf, URI_LENGTH);
    bool isBootstrap = get_bootstrap_id(lwm2mH);
    if(lwm2m_check_is_vzw_netwrok()){
        isBootstrap = get_bootstrap_id_for_vzw(securityObj, instanceId); //by joe
    }
    if (uri == NULL) return NULL;

    LOG_ARG("uri = %s", uri);

    // parse uri in the form "coaps://[host]:[port]"
    char * defaultport;
    if (0 == strncmp(uri, "coaps://", strlen("coaps://")))
    {
        host = uri+strlen("coaps://");
        defaultport = COAPS_PORT;
    }
    else if (0 == strncmp(uri, "coap://", strlen("coap://")))
    {
        host = uri+strlen("coap://");
        defaultport = COAP_PORT;
    }
    else
    {
        return NULL;
    }

    port = strrchr(host, ':');
    if (port == NULL)
    {
        port = defaultport;
    }
    else
    {
        // remove brackets
        if (host[0] == '[')
        {
            host++;
            if (*(port - 1) == ']')
            {
                *(port - 1) = 0;
            }
            else
            {
                return NULL;
            }
        }
        // split strings
        *port = 0;
        port++;
    }

    LOG_ARG("uri = %s, host = %s, port = %s",uri, host, port);

    //get psk identy

    int psk_id_len;
    char * id;
    char psk_id[125]={0};
    id = security_get_public_id(securityObj, instanceId, &psk_id_len);
    LOG_ARG("id = %s, psk_id_len = %d", id, psk_id_len);
    memcpy(psk_id, id,psk_id_len);
    //lwm2m_free(id);

    //get psk

    int keyLen;
    char * key;
    LOG_ARG("MBEDTLS_PSK_MAX_LEN = %d", MBEDTLS_PSK_MAX_LEN);
    //uint8_t psk[MBEDTLS_PSK_MAX_LEN];  //if set 32, and psk is 16Bytes, will stop in SSL/TLS handshake, why ?
    LOG_ARG("instanceId = %d", instanceId);
    key = security_get_secret_key(securityObj, instanceId, &keyLen);

    LOG_ARG("key = %s, keyLen = %d", key, keyLen);
    uint8_t psk[keyLen];
    memcpy(psk, key,keyLen);
    //lwm2m_free(key);
    //-------------------------------------------------------------

    int ret = 0, len = 0;
    unsigned char buf[256];
    get_ciphersuite_list();

    const char *pers = "dtls_client";

    mbedtls_entropy_context* entropy;
    mbedtls_ctr_drbg_context* ctr_drbg;
    mbedtls_ssl_context* ssl;
    mbedtls_ssl_config* conf;
    mbedtls_net_context* ctx;
    mbedtls_timing_delay_context* timer;

    mbedtls_ssl_session* saved_session;

    entropy = (mbedtls_entropy_context *)malloc(sizeof(mbedtls_entropy_context));
    ctr_drbg = (mbedtls_ctr_drbg_context *)malloc(sizeof(mbedtls_ctr_drbg_context));
    ssl = (mbedtls_ssl_context *)malloc(sizeof(mbedtls_ssl_context));
    memset(ssl, 0, sizeof(ssl));
    conf = (mbedtls_ssl_config *)malloc(sizeof(mbedtls_ssl_config));
    ctx = (mbedtls_net_context *)malloc(sizeof(mbedtls_net_context));
    timer = (mbedtls_timing_delay_context *)malloc(sizeof(mbedtls_timing_delay_context));
	saved_session  = (mbedtls_ssl_session *)malloc(sizeof(mbedtls_ssl_session));
    memset(saved_session, 0, sizeof(mbedtls_ssl_session));

    mbedtls_net_init(ctx);
    mbedtls_ssl_init(ssl);
    mbedtls_ssl_config_init(conf);
    mbedtls_ctr_drbg_init(ctr_drbg);

    /*
     * 0. Initialize the RNG and the session data
     */
    LOG("\n	. Seeding the random number generator...");
    mbedtls_entropy_init(entropy);
    //mbedtls_entropy_add_source(&entropy, entropy_source, NULL,
    //MBEDTLS_ENTROPY_MAX_GATHER, MBEDTLS_ENTROPY_SOURCE_STRONG);
    ret = mbedtls_ctr_drbg_seed(ctr_drbg, mbedtls_entropy_func, entropy,
                                (const uint8_t *)pers, strlen(pers));

    /*
     * 2. Start the connection
     */
    LOG_ARG(" ok\n  . Connecting to %s:%s...", host, port);
    ret = mbedtls_net_connect(ctx, host, port, MBEDTLS_NET_PROTO_UDP );
    if( ret != 0 )
    {
        LOG_ARG( " failed\n  ! mbedtls_net_connect returned %d\n\n", ret );
        #ifdef ENABLE_LWM2M_NETWORK_MANAGERMENT
        //my need to check network status
        if(lwm2m_check_is_vzw_netwrok()){
            quec_check_apn_network_available();
        }
        #endif
    }

    ret = mbedtls_net_set_nonblock(ctx );
    if( ret != 0 )
    {
        LOG_ARG( " failed\n	! net_set_(non)block() returned -0x%x\n\n",
              -ret );
    }

    /*
     * 3. Setup stuff
     */

    LOG(" ok\n  . Setting up the SSL/TLS structure...");
    ret = mbedtls_ssl_config_defaults(conf, MBEDTLS_SSL_IS_CLIENT,
                                      MBEDTLS_SSL_TRANSPORT_DATAGRAM, MBEDTLS_SSL_PRESET_DEFAULT);

    mbedtls_ssl_conf_rng(conf, mbedtls_ctr_drbg_random, ctr_drbg);
    
    LOG_ARG("psk = %s sizeof(psk)=%d", psk,sizeof(psk));
    LOG_ARG("psk_id = %s, psk_id_len = %d", psk_id, psk_id_len);
    ret = mbedtls_ssl_conf_psk(conf, psk, sizeof(psk),
                               (const uint8_t *)psk_id, psk_id_len);
    LOG_ARG("ret = %d", ret);
    if (ret != 0){
        LOG_ARG("set psk failed, ret = %x", ret);
    }
    mbedtls_ssl_set_timer_cb(ssl, timer, mbedtls_timing_set_delay, mbedtls_timing_get_delay);
    mbedtls_ssl_set_bio(ssl, ctx, mbedtls_net_send, mbedtls_net_recv, NULL );

    ret = mbedtls_ssl_setup(ssl, conf);
    LOG_ARG("ret = %d", ret);
    mbedtls_ssl_conf_handshake_timeout(conf,5000, 10000);
    mbedtls_ssl_conf_read_timeout(conf, 0);

    //int cs = mbedtls_ssl_get_ciphersuite_id("TLS-PSK-WITH-AES-128-CBC-SHA256"); //mbedtls_ssl_get_ciphersuite_id("TLS-PSK-WITH-AES-128-CCM-8");

    //mbedtls_ssl_conf_ciphersuites( &conf, &cs);
    //add by joe start
    int read_conn_info = 0;
    bool isConnColdStart = true;
	dm_server_connect_info * connect_info = (dm_server_connect_info *)malloc(sizeof(dm_server_connect_info));
    memset(connect_info, 0, sizeof(dm_server_connect_info));
    if(lwm2m_check_is_vzw_netwrok()){//add by joe
        read_conn_info = read_data_for_vzw(instanceId,connect_info, sizeof(dm_server_connect_info));
    }else{
        read_conn_info = read_data(connect_info, sizeof(dm_server_connect_info));
    }
	time_t tv_sec = lwm2m_gettime();
	if (read_conn_info == 0 && tv_sec >= 0 && lwm2m_check_is_vzw_netwrok())
	{   
	    LOG_ARG("tv_sec = %d, connect_info->lastRegistrationTime = %d, connect_info->lifetime = %d", tv_sec, connect_info->lastRegistrationTime, connect_info->lifetime);
	    if ((connect_info->lastRegistrationTime + connect_info->lifetime > tv_sec) && !Get_full_regist_flag()){
	        LOG("still within lifetime, try to recovery last registration");
			isConnColdStart = false;
	    } else {
	        LOG("data is out-of-date, delete it");
			delete_data_for_vzw(instanceId);
		}
        lwm2mH->isColdStart = isConnColdStart; //by joe set instance coldstart flag
	}
        //add by joe end
    if(lwm2mH->isColdStart || lwm2m_check_is_vzw_netwrok()){
    /*
     * 4. Handshake
     */
        LOG(" ok\n  . Performing the SSL/TLS handshake...\n");
    while ((ret = mbedtls_ssl_handshake(ssl)) != 0)//modify by joe
    //ret = mbedtls_ssl_handshake(ssl);
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            LOG_ARG(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret);
            return NULL;
        }
    }

    LOG_ARG("init sessionid = %s", ssl->session->id);
	output_buffer(stderr, ssl->session->id, 32, 0);

	if( ( ret = mbedtls_ssl_get_session(ssl, saved_session ) ) != 0 )
	{
		LOG_ARG( " failed\n	! mbedtls_ssl_get_session returned -0x%x\n\n",
						-ret );
	}

    if (isBootstrap){
           LOG("connect bootstrap server\n");
    } else {
           LOG("connect dm server, save session data\n");
           if(lwm2m_check_is_vzw_netwrok()){//add by joe
               if(lwm2mH->isColdStart){
                   save_connect_info_vzw(instanceId, lwm2mH, host, port, psk_id, psk, keyLen, saved_session);
                }else{
                   update_session_info_for_vzw(instanceId, lwm2mH, saved_session);
                }
                
           }else{ // for att
              if(write_enable){
                  save_connect_info(lwm2mH, host, port, psk_id, psk, keyLen, saved_session);
              }
            }
          }
} else {
        //use last time connect info
    //LOG_ARG("connect_info->saved_session = %s", connect_info->saved_session);

	output_buffer(stderr, (dm_server_connect_info*)connect_info->saved_session.id, 32, 0);

        LOG("joe mbedtls_ssl_session_reset\n");
    if( ( ret = mbedtls_ssl_session_reset(ssl) ) != 0 )
    {
         LOG_ARG( " failed\n	! mbedtls_ssl_session_reset returned -0x%x\n\n",
								-ret );
    }

        LOG("joe mbedtls_ssl_set_session\n");
    if( ( ret = mbedtls_ssl_set_session(ssl, &(connect_info->saved_session))) != 0 )
    {
        LOG_ARG( " failed\n	! mbedtls_ssl_conf_session returned %d\n\n",
								ret );
    }

	//memcpy( ssl->session_negotiate, saved_session, sizeof(mbedtls_ssl_session));
        LOG("joe mbedtls_ssl_handshake \n");
    while ((ret = mbedtls_ssl_handshake(ssl)) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            LOG_ARG(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret);
                return NULL;
        }
    }

    LOG_ARG("reconnect sessionid = %s", ssl->session->id);
	output_buffer(stderr, ssl->session->id, 32, 0);
        if(lwm2m_check_is_vzw_netwrok()){
            update_session_info_for_vzw(instanceId, lwm2mH, &(connect_info->saved_session));
        }else{ // for att
         if(write_enable){
            save_connect_info(lwm2mH, host, port, psk_id, psk, keyLen, &(connect_info->saved_session));
         }
        }

    LOG( " ok\n" );
}

    /*
     * 4. Handshake
     * 5. init dtls_connection_t
     */
    LOG(" ok\n  . init dtls_connection_t...\n");

    connP = (dtls_connection_t *)malloc(sizeof(dtls_connection_t));
    if (connP != NULL)
    {
        memset(connP, 0, sizeof(dtls_connection_t));
        connP->sock = sock;
        //memcpy(&(connP->addr), addr, addrLen);
        //connP->addrLen = addrLen;
        connP->next = connList;

        //connP->dtlsSession = (session_t *)malloc(sizeof(session_t));
        //memset(connP->dtlsSession, 0, sizeof(session_t));
        //connP->dtlsSession->addr.sin6 = connP->addr;
        //connP->dtlsSession->size = connP->addrLen;
        connP->lastSend = lwm2m_gettime();
        connP->last_conn_time = lwm2m_gettime();//add by joe
        connP->dlts_resumption = 0;
        connP->securityObj = securityObj;
        connP->securityInstId = instanceId;
        connP->lwm2mH = lwm2mH;
        connP->net_ctx= ctx;

        if (security_get_mode(connP->securityObj,connP->securityInstId)
            != LWM2M_SECURITY_MODE_NONE)
        {
            connP->ssl_ctx = ssl;
        }
    }
    LOG("connection_create E");
    return connP;
}

int connection_send(dtls_connection_t *connP, uint8_t * buffer, size_t length){
    LOG("connection_send\r\n");
    //dump_stack("mango");
    int ret, len;
    LOG_ARG("connection_send buffer = %s, length = %d by securityInstId=%d sessionID=%d \n", buffer, length,connP->securityInstId,connP->ssl_ctx->session->id);
    
    do {
        ret = mbedtls_ssl_write(connP->ssl_ctx, (const uint8_t *)buffer, length);
    } while (ret == MBEDTLS_ERR_SSL_WANT_READ ||
             ret == MBEDTLS_ERR_SSL_WANT_WRITE);
    len = ret;
    LOG_ARG( " %d bytes written\n\n%s\n", len, buffer);

    return 0;
}

void connection_free(dtls_connection_t * connList)
{
    while (connList != NULL)
    {
        dtls_connection_t * nextP;

        nextP = connList->next;
        mbedtls_ssl_close_notify(connList->ssl_ctx);
        mbedtls_printf(" . Closing the connection ... done\n");


        mbedtls_net_free(connList->net_ctx);
        mbedtls_ssl_free(connList->ssl_ctx);
        mbedtls_ssl_config_free(connList->ssl_conf);
        mbedtls_ctr_drbg_free(connList->ctr_drbg_ctx);
        mbedtls_entropy_free(connList->entropy_ctx);

        connList = nextP;
    }
}

int connection_handle_packet(dtls_connection_t *connP, uint8_t * buffer, size_t numBytes){
    LOG("connection_handle_packet X");

    // no security, just give the plaintext buffer to liblwm2m
    lwm2m_handle_packet(connP->lwm2mH, buffer, numBytes, (void*)connP);
    LOG("connection_handle_packet E");
    return 0;
}

void resumption_last_connection_free(dtls_connection_t * connList)
{
    if(connList->net_ctx != NULL){
        LOG("resumption_last_connection_free net_ctx\n");
        mbedtls_net_free(connList->net_ctx);
    }
    if(connList->ssl_ctx != NULL){
        LOG("resumption_last_connection_free ssl_ctx\n");
        mbedtls_ssl_free(connList->ssl_ctx);
    }
}
#endif


uint8_t lwm2m_buffer_send(void * sessionH,
                          uint8_t * buffer,
                          size_t length,
                          void * userdata)
{
    dtls_connection_t * connP = (dtls_connection_t*) sessionH;

    if (connP == NULL)
    {
        fprintf(stderr, "#> failed sending %lu bytes, missing connection\r\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR ;
    }

    if (-1 == connection_send(connP, buffer, length))
    {
        fprintf(stderr, "#> failed sending %lu bytes\r\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR ;
    }

    return COAP_NO_ERROR;
}

bool lwm2m_session_is_equal(void * session1,
                            void * session2,
                            void * userData)
{
    return (session1 == session2);
}
